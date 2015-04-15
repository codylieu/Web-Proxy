#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "proxy.h"

struct LRU_Cache {
  // Map
  // Doubly Linked List
}
typedef struct LRU_Cache cache

// Also need a list of connections
struct connections {
  int timeout;
  int closed;
}

void get () {

}

void set () {

}

void removeNode () {

}

void setHead () {

}

// Called by pthread to process each new connection request
void processRequest () {

}

// Close connection to the web server and the connection to the browser 
// when there is no more data to be transferred
void closeConnection () {

}

int main(int argc, char ** argv) {
  
}