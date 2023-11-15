#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <pcap.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "ip/ip.h"
#include "ip/arp.h"
#include "ip/rip.h"

#include "tcp/tcp.h"
#include "tcp/connection.h"


char ss[100000];

int main() {
  struct sockaddr_in cliaddr, servaddr;
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);

  printf("listenfd = %d\n", listenfd);

  int optval;
  int rv;
  int connfd;
  int loop;
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(10086);


  bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
  optval = 1;
  rv = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                      (const void *) &optval, sizeof(int));
  if (rv < 0) {
    printf("%s\n", strerror(errno));
  }
  int listen_res = listen(listenfd, SOMAXCONN);

  printf("listen = %d\n", listen_res); 

  socklen_t clilen = sizeof(cliaddr);
  connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
  printf("new connection, connfd = %d\n", connfd);

#define MAXLINE 4096
  ssize_t n;
  char buf[MAXLINE];
  size_t acc = 0;
  while ((n = read(connfd, buf, MAXLINE)) > 0) {
    acc += n;
    write(1, buf, n);
    printf("\n");
  }
  printf("acc = %d\n", acc);


  int close_res = close(connfd);

  printf("close = %d\n", close_res);


  return 0;
}