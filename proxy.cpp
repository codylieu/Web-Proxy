#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <map>  //std::map library to use for caching
#include <iterator>

#include "proxy.h"

#define MAX_MSG_LENGTH 8192
#define MAX_BACK_LOG 5
#define MAX_ATTEMPTS 5

struct node {
  char *val;
  struct node *next;
  struct node *prev;
}*root;

struct LRU_Cache {
  // Map
  std::map<char, node> nodeMap;
  // Doubly Linked List
  node *head;
  node *end;
  int capacity;
  int size;
};
// Global Cache
LRU_Cache cache;

// Also need a list of connections
struct connections {
  int timeout;
  int closed;
};

struct thread_params {
  uint16_t port;
  int sockfd;
  char buf[MAX_MSG_LENGTH];
};

//********** METHODS FOR CACHING **********//

void removeNode (node *node) {

}

void setHead (node *node) {

}

void get (char *key) {
  // If you can't find the key...
  if(cache.nodeMap.find(*key) == cache.nodeMap.end())  {
    return;
  }
  // If you can find the key
  else  {
    node latest = cache.nodeMap.find(*key)->second;
    removeNode(&latest);
    setHead(&latest);
  }
  
}

void set () {

}

// Close connection to the web server and the connection to the browser 
// when there is no more data to be transferred
void closeConnection () {

}

void addToCache (node *n) {
  // printf("node value %s\n", n->val);
  // If you can't find the key
  if(cache.nodeMap.find(*n->val) == cache.nodeMap.end())  {
    // This will change once we figure out how size of the cache works
    if(cache.size < cache.capacity)  {
      setHead(n);
      cache.nodeMap.insert(std::pair<char, node>(n->val,n));
    }

  }
  // If you can find the key
  else  {
    node latest = cache.nodeMap.find(*n->val)->second;

  }
}

int cacheContains () {
  return 0;
}

//Called by process request 
void sendResponse (char *url, uint16_t port, int sockfd, char *httpVer) {
  struct addrinfo hints, *res;
  int status;
  char ipstr[INET_ADDRSTRLEN];

  memset(&ipstr, 0, sizeof(ipstr));
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(strtok(url, "/"), NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return;
  }
  printf("URL Check: %s\n", url);
  printf("HTTP Check: %s\n", httpVer);
  void *addr;
  char ipver[] = "IPv4";

  // Fix this segfault
  struct sockaddr_in *ipv4 =(struct sockaddr_in *)res->ai_addr;
  addr = &(ipv4->sin_addr);
  inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));

  // IP Address is stored in ipstr
  printf("IP Address Check: %s\n", ipstr);

  if (cacheContains()) {
    return;
  }
  
  // Not in cache
  int responsefd, count;

  // Keep trying to connect until it connects or max attempts is reached
  // while (1) { // Do I need this while loop now that I remembered the return?
  //   if (count > MAX_ATTEMPTS) {
  //     printf("Couldn't connect\n");
  //     return;
  //   }
  //   if ((responsefd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
  //     printf("Socket call failed\n");
  //     continue;
  //   }
  //   struct sockaddr_in sin;
  //   memset(&sin, 0, sizeof(sin));
  //   sin.sin_family = AF_INET;
  //   memcpy(&sin.sin_addr.s_addr, ipstr, INET_ADDRSTRLEN);
  //   sin.sin_port = ipv4->sin_port;

  //   if (connect(responsefd, (sockaddr *)&sin, sizeof(sin)) < 0) {
  //     printf("Connect call failed\n");
  //     count++;
  //     continue;
  //   }
  //   break;
  // }
  // printf("Connection Achieved\n");

  // char response[MAX_MSG_LENGTH];
  // sprintf(response, "%s %s %s", "GET", url, httpVer);

  // ssize_t written;
  // if ((written = write(responsefd, response, sizeof(response))) <= 0) {
  //   printf("Write failed\n");
  //   return;
  // }

  // int numBytes = 0;
  // while (strcmp(response, "\r\n") > 0) {
  //   memset((void *)response, 0, MAX_MSG_LENGTH);

  //   if ((numBytes = recv(responsefd, response, MAX_MSG_LENGTH, 0)) < 0) {
  //     printf("Recv didn't receive anything\n");
  //     return;
  //   }

  //   if (strcmp(response, "\r\n") == 0) {
  //     // close connection
  //     closeConnection();
  //   }

  //   send(sockfd, response, MAX_MSG_LENGTH, 0);
  // }

  freeaddrinfo(res);

}

// Called by pthread to process each new connection request
void *processRequest (void *input) {
  uint16_t port;
  int sockfd;
  char buf[MAX_MSG_LENGTH];
  memset(buf, 0, MAX_MSG_LENGTH);

  struct thread_params *params = (struct thread_params *)input;
  port = params->port;
  sockfd = params->sockfd;
  memcpy(buf, params->buf, MAX_MSG_LENGTH);
  char *tok = strtok(buf, " ");

  if (strcmp(tok, "GET") != 0) { // Only need to deal with GET requests
    printf("Command is not get\n");
    return NULL;
  }

  // Command is get
  // get url and then use getaddrinfo() to get IP Address

  tok = strtok(NULL, " "); // In the form http://www.cnn.com/
  char *url = tok + 7;
  printf("URL: %s\n", url);

  tok = strtok(NULL, " ");
  char *httpVer = strtok(tok, "\n");
  // printf("HTTP Version: %s\n", httpVer);

  sendResponse(url, port, sockfd, httpVer);

  // change LRU Cache as necessary
  // Should I initialize it in main?

  // New node to add to cache
  node n;
  n.val = url;

  addToCache(&n);

  return NULL;
}

int initServer (uint16_t port) {
  struct sockaddr_in sin;
  socklen_t addr_size;
  int s, new_s;
  char buf[MAX_MSG_LENGTH];

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

    if ((addr_size = recv(new_s, buf, sizeof(buf), 0)) > 0) {
      struct thread_params params;
      params.port = port;
      params.sockfd = new_s;
      memset(params.buf, 0, MAX_MSG_LENGTH);
      memcpy(params.buf, buf, MAX_MSG_LENGTH);

      pthread_t requestThread;
      pthread_create(&requestThread, NULL, &processRequest, (void *)&params);
    }

    close(new_s);
  }
  close(s);
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

  cache.capacity = cacheSize;
  cache.size = 0;

  return initServer(port);
}


















