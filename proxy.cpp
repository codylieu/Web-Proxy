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
#include <iostream>
#include <signal.h>

#include "proxy.h"

#define MAX_MSG_LENGTH 8192
#define MAX_BACK_LOG 5
#define MAX_ATTEMPTS 5

struct node {
  char *val; // Should probably be a char *
  int size;
  node *next;
  node *prev;
};

struct LRU_Cache {
  // Map
  std::map<char*, node> nodeMap;
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
  int sockfd;
  char buf[MAX_MSG_LENGTH];
};

//********** METHODS FOR CACHING **********//

void removeNode (node *n) {
  node *cur = n;
  node *pre = cur->prev;
  node *post = cur->next;
  
  if(pre != NULL) {
    pre->next = post;
  }
  else  {
    cache.head = post;
  }

  if(post != NULL)  {
    post->prev = pre;
  }
  else  {
    cache.end = pre;
  }

}

void setHead (node *n) {
  n->next = cache.head;
  n->prev = NULL;
  if(cache.head != NULL)  {
    printf("head: %s\n",cache.head->val);
    cache.head->prev = n;
  }
  cache.head = n;
  if(cache.end == NULL)  {
    cache.end = n;
  }
}

void get (char *key) {
  // If you can't find the key...
  if(cache.nodeMap.find(key) == cache.nodeMap.end())  {
    return;
  }
  // If you can find the key
  else  {
    node latest = cache.nodeMap.find(key)->second;
    removeNode(&latest);
    setHead(&latest);
  }
  
}

// Close connection to the web server and the connection to the browser 
// when there is no more data to be transferred
void closeConnection () {

}

void addToCache (node *n) {
  // printf("node value %s\n", n->val);
  // If the key isn't in the map...
  if(cache.nodeMap.find(n->val) == cache.nodeMap.end())  {
    // If the cache isn't full, just add the new node
    if(cache.size < cache.capacity)  {
      setHead(n);
      cache.nodeMap.insert(std::pair<char*, struct node>(n->val,*n));
      cache.size++;
    }
    // If the cache is full, need to kick off the least recently used
    else  {
      cache.nodeMap.erase(n->val);
      cache.end = cache.end->prev;
      if(cache.end != NULL) {
        cache.end->next = NULL;
      }
      setHead(n);
      cache.nodeMap.insert(std::pair<char*, struct node>(n->val,*n));
    }
  }
  // If the key is in the map...
  else  {
    // Set the key/node pair as the most recently used
    node latest = cache.nodeMap.find(n->val)->second;
    removeNode(&latest);
    setHead(&latest);
  }

  // The following code is just to print out the current cache
  printf("CURRENT CACHE:\n");
  node *cur = cache.head;
  while(cur != NULL)  {
    printf(" %s ",cur->val);
    cur = cur->prev;
  }
  printf("\n");

}

int cacheContains () {
  return 0;
}

void handleResponse (int clientfd, char *originalRequest, char *ipstr, uint16_t serverPort) {
  if (cacheContains()) {
    return;
  }
  // Not in cache
  int serverfd = -1;
  int count = 0;
  while (serverfd < 0 && count < MAX_ATTEMPTS) {
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    count++;
  }
  if (serverfd < 0) {
    printf("Socket call failed\n");
    return;
  }
  if (serverfd > 0) {
    printf("Socket created\n");
  }

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_addr.s_addr = inet_addr(ipstr);
  sin.sin_family = AF_INET;
  sin.sin_port = serverPort;

  int count2 = 0;
  while (count2 < MAX_ATTEMPTS) {
    if (connect(serverfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      perror("Connect error");
      count2++;
      continue;
    }
    printf("Connection Achieved\n");
    break;
  }

  send(serverfd, originalRequest, strlen(originalRequest), 0);

  char response[MAX_MSG_LENGTH];
  int numBytes = 0;
  do {
    memset(response, 0, MAX_MSG_LENGTH);
    while (1) {
      if ((numBytes = recv(serverfd, response, MAX_MSG_LENGTH, 0)) < 0) {
        continue;
      }
      break;
    }
    printf("===== Server response: %s\n", response);
    send(clientfd, response, numBytes, 0);
  }
  while (numBytes > 0);

  // Need to send Original Request line by line?

  // char *token = strtok(originalRequest, "\n");
  // printf("Token: %s\n", token);
  // char toSend2[MAX_MSG_LENGTH];
  // memset(toSend2, 0, MAX_MSG_LENGTH);
  // strcpy(toSend2, strdup(token));
  // strcat(toSend2, "\n");
  // send(serverfd, toSend2, strlen(toSend2), 0);
  // // printf("Modified original request: %s\n", token);

  // while (strcmp(token, "\r\n") > 0) {
  //   token = strtok(NULL, "\n");
  //   if (strstr(token, "Keep-Alive:") || strstr(token, "Proxy-Connection: ") || strstr(token, "Connection: ")) {
  //     // Do nothing
  //   }
  //   // if (0) {

  //   // }
  //   else {
  //     printf("Token: %s\n", token);
  //     if (strcmp(token, "\r\n") == 0) {
  //       char close[MAX_MSG_LENGTH];
  //       memset(close, 0, MAX_MSG_LENGTH);
  //       sprintf(close, "Connection: close\r\n");
  //       printf("Token: %s\n", close);
  //       send(serverfd, close, strlen(close), 0);
  //     }
  //     char toSend[MAX_MSG_LENGTH];
  //     memset(toSend, 0, MAX_MSG_LENGTH);
  //     strcpy(toSend, strdup(token));
  //     strcat(toSend, "\n");
  //     send(serverfd, toSend, strlen(toSend), 0);
  //   }
  // }

  close(serverfd);
  close(clientfd);
  return;
}

// Called by pthread to process each new connection request
void *processRequest (void *input) {
  int sockfd;
  char buf[MAX_MSG_LENGTH], originalRequest[MAX_MSG_LENGTH];
  memset(buf, 0, MAX_MSG_LENGTH);
  memset(originalRequest, 0, MAX_MSG_LENGTH);

  struct thread_params *params = (struct thread_params *)input;
  sockfd = params->sockfd;
  memcpy(buf, params->buf, MAX_MSG_LENGTH);
  memcpy(originalRequest, params->buf, MAX_MSG_LENGTH);
  char *tok = strtok(buf, " ");

  if (strcmp(tok, "GET") != 0) {
    printf("Command is not get\n");
    return NULL;
  }

  tok = strtok(NULL, " "); // In the form http://www.cnn.com/
  char *url = tok + 7;

  struct addrinfo hints, *res;
  int status;
  char ipstr[INET_ADDRSTRLEN];

  memset(&ipstr, 0, sizeof(ipstr));
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(strtok(url, "/"), "http", &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return NULL;
  }

  void *addr;
  char ipver[] = "IPv4";

  struct sockaddr_in *ipv4 =(struct sockaddr_in *)res->ai_addr;
  addr = &(ipv4->sin_addr);
  inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));

  handleResponse(sockfd, originalRequest, ipstr, ipv4->sin_port);

  // New node to add to cache
  node n;
  n.val = url;
  addToCache(&n);

  freeaddrinfo(res);
  return NULL;
}

int initServer (uint16_t port) {
  struct sockaddr_in sin;
  socklen_t addr_size;
  int s, new_s;
  char buf[MAX_MSG_LENGTH];
  memset(buf, 0, MAX_MSG_LENGTH);

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

    if ((addr_size = recv(new_s, buf, MAX_MSG_LENGTH, 0)) > 0) {
      struct thread_params params;
      memset(&params, 0, sizeof(params));
      params.sockfd = new_s;
      memset(params.buf, 0, MAX_MSG_LENGTH);
      memcpy(params.buf, buf, MAX_MSG_LENGTH);

      pthread_t requestThread;
      pthread_create(&requestThread, NULL, &processRequest, (void *)&params);
    }

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
  signal(SIGPIPE, SIG_IGN);

  uint16_t port = atoi(argv[1]);
  int cacheSize = atoi(argv[2]);

  cache.capacity = cacheSize;
  cache.size = 0;
  cache.head = NULL;
  cache.end = NULL;

  return initServer(port);
}
