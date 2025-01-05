#!/bin/sh
g++ -pthread server.cpp -o server
g++ client.cpp -o client

chmod 755 server
chmod 700 client
