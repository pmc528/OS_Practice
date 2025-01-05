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
#include <thread>
#include <pthread.h>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

//cThread class used to initialize serverSD and databuf and later use this to instantiate cThread object
class cThread {
public:
     cThread() {}
     int serverSD;
     char * databuf;
};

//startRoutine necessary for the pthread_create call.
//creates new thread and also will send the readCalls # to client side
void *startRoutine (void * arg) {
    cThread param = *(cThread *) arg;
    //accept new connection
    //recieve a message from client with # of iterations to perform
  int repetition;
    read(param.serverSD, &repetition, sizeof(repetition));

    //read from client the appropriate # of iterations of BUFFSIZE amounts of data
   int readCalls = 0;
   int readThisRep;
   int set = 0;
   while(set < repetition) {
     readThisRep = 0;
     while (readThisRep < BUFFSIZE) {
       int bytesRead = read(param.serverSD, param.databuf, BUFFSIZE - readThisRep);
       readThisRep += bytesRead;
       readCalls++;
     }
       set++;
   }
    //send readCalls # to client
    write(param.serverSD, &readCalls, sizeof(readCalls));

    //close connection
    close(param.serverSD);

    //terminate thread
    pthread_exit(NULL);
}

//main method will join to socket and create new thread for each respective client request
//thread creation done in a continuous loop
int main(int argc, char *argv[])
{
    //allocate dataBuf[BUFFSIZE], where BUFFSIZE = 1500 to read in data being sent by client
    char databuf[BUFFSIZE];
    bzero(databuf, BUFFSIZE);

    //build address
    int port = atoi(argv[1]);
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(port);

    //open socket
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;

    //bind to socket
    int rc;
    if (bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress)) < 0)
    {
        cerr << "Bind Failed" << endl;
    }

    //the use the listen() system call to tell the server how many concurrent connections to listen for
    listen(serverSD, NUM_CONNECTIONS);       //setting number of pending connections

    //Dispatch loop
     for(;;) { //infinite for loop
 
        //create new socket
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
        cout << "Accepted Socket #: " << newSD <<endl;

        //create new data buffer
        char newDatabuff[BUFFSIZE];
        memset(newDatabuff, '\0', sizeof(newDatabuff[0]) * BUFFSIZE);

        //create param to pass to thread creation
        cThread * param = new cThread();
        param->databuf = newDatabuff;
        param->serverSD = newSD;

        //create thread
    pthread_t serviceThread;
    pthread_create( &serviceThread, NULL, startRoutine, (void *)param); //edit

    //separate thread of execution from thread object
    pthread_detach(pthread_self());
    }
 
    close(serverSD); //close
    return 0;
}




