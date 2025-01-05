//Peter Chang
//CSS 503: Program 4
//6/8/2022
//This assignment is intended for three purposes: (1) to utilize various socket-related system calls,
//(2) to create a multi-threaded server and (3) to evaluate the throughput of different mechanisms
//when using TCP/IP do to do point-to-point communication over a network.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

const int BUFFSIZE=1500;

//main function will make connection with server and measure the write system calls. Must have the server code running before running this.
int main(int argc, char *argv[])
{
    //check num of args
    if (argc != 7) {
       cerr << "Usage: " << argv[0] << "serverName" << endl;
       return -1;
    }

    //server name: csslab3.uwb.edu
    char * serverName = argv[1];
    
    //for port use last 5 digits of student ID
    char * serverPort = argv[2];
    
    //# of repetitions
    int repetition = atoi(argv[3]);
    
    //# of data buffers
    int nbuffs = atoi(argv[4]);
    
    //data buffer size in bytes
    int buffsize = atoi(argv[5]);

    //transfer scenario...pick between 1 2 or 3
    int type = atoi(argv[6]);

    //check args and print out error message if args are invalid
    if (repetition < 0 || nbuffs < 1 || buffsize < 1 || type < 1 || type > 3) {
        cout << "Error: One of your args is invalid." << endl;
    }
    
//    cout << "Flag1" << endl; //debugging
    
    //intialize addrinfo variables
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;                          /* Allow IPv4 or IPv6*/
    hints.ai_socktype = SOCK_STREAM;                    /* TCP */
    hints.ai_flags = 0;                                     /* Optional Options*/
    hints.ai_protocol = 0;                                 /* Allow any protocol*/
   
    /* Use getaddrinfo() to get addrinfo structure corresponding to serverName / Port
       This addrinfo structure has internet address which can be used to create a socket too*/
    int rc = getaddrinfo(serverName, serverPort, &hints, &result);
    if (rc != 0) {
       cerr << "ERROR: " << gai_strerror(rc) << endl;
       exit(EXIT_FAILURE);
    }

    //Iterate through addresses and connect
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        //create endpoint communication
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1) {
            continue;
        }
        
        //A socket has been successfully created
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0) {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        } else {
            //break from while loop
            break;
        }
    }

    //if no valid address was returned after iterating thru if loop, exit
    if (rp == NULL) {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    else
        {
            cout << "Client Socket: " << clientSD << endl;
        }
    
    //free address data from memory
    freeaddrinfo(result);

    //send the # of repetition over to the server
    write(clientSD, &repetition, sizeof(repetition));
        
    //create data buffer and make null
    char databuf[nbuffs][buffsize];
    memset(databuf, '\0', sizeof(databuf[0][0]) * nbuffs * buffsize);

    //initialize chrono variables and start timers before the tests
    time_point<chrono::steady_clock> start, end;
    start = chrono::steady_clock::now();
    
    //perform tests w/ server & measure time using chrono library
    
    int iter = 0;
    while(iter < repetition) {
        switch(type) {
            case 1: {
                int ii = 0;
                while(ii < nbuffs){
                     write(clientSD, databuf[ii], buffsize);
                     ii++;
                 }
                break;
            }

            case 2: {
                struct iovec vector[nbuffs];
                int k = 0;
                while(k < nbuffs){
                    vector[k].iov_base = databuf[k];
                    vector[k].iov_len = buffsize;
                    k++;
                }
                 writev(clientSD, vector, nbuffs);
                break;
            }
                
            case 3: {
                write(clientSD, databuf, nbuffs * buffsize);
                break;
            }
        }
        iter++;
    }
    
    //end time
    end = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    
    //receive message from server
    int numberOfReads;
    read(clientSD, &numberOfReads, sizeof(numberOfReads));
    
    //print out the summary info
      cout << "[Test: type ="<< type<<", nbuffs = " << nbuffs << ", buffsize = " << buffsize << "]: ";
      cout << "time = " << elapsed_seconds.count()*1000000 << "usec,";
      cout << "num of reads = " << numberOfReads << ",";
      cout << "throughput = " << ((buffsize * repetition * 8 * nbuffs) * 0.000000001)/elapsed_seconds.count() << "gbps" << endl;;
    
    //close socket connection
    close(clientSD);
    return 0;
}




