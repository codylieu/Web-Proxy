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
#include <signal.h>

#include "proxy.h"

#define MAX_MSG_LENGTH 8192
#define MAX_BACK_LOG 5
#define MAX_ATTEMPTS 5

struct LRU_Cache {
  // Map
  // Doubly Linked List
  int size;
};
typedef struct LRU_Cache cache;

struct node {
  int val; // Should probably be a char *
  int size;
  struct node *next;
  struct node *prev;
}*root;

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

void get () {

}

void set () {

}

void removeNode () {

}

void setHead () {

}

// Close connection to the web server and the connection to the browser 
// when there is no more data to be transferred
void closeConnection () {

}

void addToCache () {

}

int cacheContains () {
  return 0;
}

void sendResponse (char *url, uint16_t port, int sockfd, char *httpVer, char *originalRequest) {
  struct addrinfo hints, *res;
  int status;
  char ipstr[INET_ADDRSTRLEN];

  memset(&ipstr, 0, sizeof(ipstr));
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(strtok(url, "/"), "http", &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return;
  }

  // printf("URL Check: %s\n", url);
  // printf("HTTP Check: %s\n", httpVer);

  void *addr;
  char ipver[] = "IPv4";

  struct sockaddr_in *ipv4 =(struct sockaddr_in *)res->ai_addr;
  addr = &(ipv4->sin_addr);
  inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));

  // IP Address is stored in ipstr
  // printf("IP Address Check: %s\n", ipstr);

  if (cacheContains()) {
    return;
  }
  // Not in cache
  int responsefd = -1;
  int count = 0;
  while (responsefd < 0 && count < MAX_ATTEMPTS) {
    responsefd = socket(AF_INET, SOCK_STREAM, 0);
    count++;
  }
  if (responsefd < 0) {
    printf("Socket call failed\n");
    return;
  }
  if (responsefd > 0) {
    printf("Socket created\n");
  }

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_addr.s_addr = inet_addr(ipstr);
  sin.sin_family = AF_INET;
  sin.sin_port = ipv4->sin_port;

  int count2 = 0;
  while (count2 < MAX_ATTEMPTS) {
    if (connect(responsefd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      perror("Connect error");
      count2++;
      continue;
    }
    printf("Connection Achieved\n");
    break;
  }

  // Need to send Original Request line by line?

  // char *token = strtok(originalRequest, "\n");
  // printf("Token: %s\n", token);
  // char toSend2[MAX_MSG_LENGTH];
  // memset(toSend2, 0, MAX_MSG_LENGTH);
  // strcpy(toSend2, strdup(token));
  // strcat(toSend2, "\n");
  // send(responsefd, toSend2, strlen(toSend2), 0);
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
  //       send(responsefd, close, strlen(close), 0);
  //     }
  //     char toSend[MAX_MSG_LENGTH];
  //     memset(toSend, 0, MAX_MSG_LENGTH);
  //     strcpy(toSend, strdup(token));
  //     strcat(toSend, "\n");
  //     send(responsefd, toSend, strlen(toSend), 0);
  //   }
  // }

  send(responsefd, originalRequest, strlen(originalRequest), 0);

  char response[MAX_MSG_LENGTH];
  int numBytes = 0;
  do {
    memset(response, 0, MAX_MSG_LENGTH);
    while (1) {
      if ((numBytes = recv(responsefd, response, MAX_MSG_LENGTH, 0)) < 0) {
        continue;
      }
      break;
    }
    printf("===== Server response: %s\n", response);
    send(sockfd, response, numBytes, 0);
  }
  while (numBytes > 0);

  freeaddrinfo(res);
  close(responsefd);
  close(sockfd);
  return;
}

// Called by pthread to process each new connection request
void *processRequest (void *input) {
  uint16_t port;
  int sockfd;
  char buf[MAX_MSG_LENGTH], originalRequest[MAX_MSG_LENGTH];
  memset(buf, 0, MAX_MSG_LENGTH);
  memset(originalRequest, 0, MAX_MSG_LENGTH);

  struct thread_params *params = (struct thread_params *)input;
  port = params->port;
  sockfd = params->sockfd;
  memcpy(buf, params->buf, MAX_MSG_LENGTH);
  memcpy(originalRequest, params->buf, MAX_MSG_LENGTH);
  char *tok = strtok(buf, " ");

  if (strcmp(tok, "GET") != 0) { // Only need to deal with GET requests
    printf("Command is not get\n");
    return NULL;
  }

  // Command is get
  // get url and then use getaddrinfo() to get IP Address

  tok = strtok(NULL, " "); // In the form http://www.cnn.com/
  char *url = tok + 7;
  // printf("URL: %s\n", url);

  tok = strtok(NULL, " ");
  char *httpVer = strtok(tok, "\n");
  // printf("HTTP Version: %s\n", httpVer);

  sendResponse(url, port, sockfd, httpVer, originalRequest);

  // change LRU Cache as necessary
  // Should I initialize it in main?

  // addToCache();

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
      params.port = port;
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

  return initServer(port);
}


















