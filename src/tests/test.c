#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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

  char *addr = "10.100.1.2";

  int sockfd;
  struct sockaddr_in servaddr;
  FILE* fp;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  printf("sockfd = %d\n", sockfd);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(10086);
  inet_pton(AF_INET, addr, &servaddr.sin_addr);

  int conn_res = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  printf("connect = %d\n", conn_res);

  char *s = "Hello world!";

  int write_res = write(sockfd, s, strlen(s));

  printf("write = %d\n", write);

  int close_res = close(sockfd);

  printf("close = %d\n", close_res);


  return 0;
}