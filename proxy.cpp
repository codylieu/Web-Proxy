#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>

#include "proxy.h"

#define MAX_MSG_LENGTH 500
#define MAX_BACK_LOG 5

struct LRU_Cache {
  // Map
  // Doubly Linked List
};
typedef struct LRU_Cache cache;

// Also need a list of connections
struct connections {
  int timeout;
  int closed;
};

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

int initServer (uint16_t port) {
  struct sockaddr_in sin;
  socklen_t addr_size;
  char buf[MAX_MSG_LENGTH];
  int s, new_s;

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  listen(s, MAX_BACK_LOG);
  while (1) {
    addr_size = sizeof(sin);
    if ((new_s = accept(s, (struct sockaddr *)&sin, &addr_size)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    socklen_t len = recv(new_s, buf, sizeof(buf), 0);
    printf("CHECK: %d\n", len);
    if ((addr_size = len) > 0) {
      printf("REQUEST RECEIVED: %s\n", buf);
    }
    close(new_s);
  }
  return 0;
}

int main(int argc, char ** argv) {
  if (argc < 3) {
    printf("Too few arguments\n");
    return 1;
  }
  if (argc > 3) {
    printf("Too many arguments\n");
    return 1;
  }
  // signal(SIGPIPE, ignore);

  uint16_t port = atoi(argv[1]);
  int cacheSize = atoi(argv[2]);

  return initServer(port);
}


















