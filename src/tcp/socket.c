#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "ethernet/device.h"

#include "tcp.h"
#include "connection.h"
#include "socket.h"
#include "netstack.h"

extern struct in_addr dst;
extern struct in_addr src;
extern uint16_t dst_port;
extern uint16_t src_port;

extern enum STATE state;

extern uint32_t receiver_seqno;
extern uint32_t receiver_ackno;
extern uint8_t sender_buffer[BUFFER_SIZE];
extern uint8_t receiver_buffer[BUFFER_SIZE];
extern int receiver_receivedData[BUFFER_SIZE];

typedef struct SocketListNode {
  int sockfd;
  uint16_t src_port;
  struct sockaddr *addr;
  struct SocketListNode *next;
} SocketListNode;

SocketListNode *head;

int read_offset = 0;
int write_offset = 0;
static int _socket_fd = 100;
static uint16_t _src_port = 10000;

extern int stack_set;

static enum STATE init_state = CLOSED;

int __wrap_socket(int domain, int type, int protocol) {
  if (!stack_set)
    setupNetStack();
  if (domain != AF_INET || type != SOCK_STREAM || protocol != 0)
    return -1;
  SocketListNode *tmp = malloc(sizeof(SocketListNode));
  tmp->sockfd = ++_socket_fd;
  tmp->addr = NULL;
  tmp->next = head;
  head = tmp;
  return tmp->sockfd;
}

int __wrap_listen(int socket, int backlog) {
  // we only supply single connection, so socket is, well, UNUSED
  state = LISTEN;
  init_state = LISTEN;
  return 0;
}

int __wrap_bind(int socket, const struct sockaddr *address, socklen_t address_len) {
  for (SocketListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->sockfd == socket) {
      tmp->addr = malloc(sizeof(uint8_t) * address_len);
      memcpy(tmp->addr, address, sizeof(uint8_t) * address_len);
      return 0;
    }
  }
  return -1;
}

int __wrap_connect(int socket, const struct sockaddr *address, socklen_t address_len) {
  // printf("__wrap_connect\n");
  for (SocketListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->sockfd == socket) {
      tmp->addr = malloc(sizeof(uint8_t) * address_len);
      memcpy(tmp->addr, address, sizeof(uint8_t) * address_len);
      tmp->src_port = ++_src_port;

      const struct sockaddr_in *sin = (struct sockaddr_in *)address;
      uint16_t dst_port = ntohs(sin->sin_port);
      struct in_addr dst = sin->sin_addr;
      struct in_addr src = *getLocalIP(0);

      int start_time = clock();      

      while (1) {
        if ((double)(clock() - start_time) / CLOCKS_PER_SEC > 10.0)
          return -1;
        // printf("## dst_port = %d, src_port = %d\n", dst_port, tmp->src_port);
        if (activeEstablish(dst, src, dst_port, tmp->src_port) == 0) {
          read_offset = 0;
          write_offset = 0;
          return 0;
        }
        sleep(1);
      }
    }
  }
  return -1;
}

int __wrap_accept(int socket, struct sockaddr *address, socklen_t *address_len) {
  // printf("__wrap_accept\n");
  while (state != ESTABLISHED) {}
  struct sockaddr_in *sin = (struct sockaddr_in *)address;
  sin->sin_port = dst_port;
  sin->sin_addr = dst;
  sin->sin_family = AF_INET;
  *address_len = sizeof(struct sockaddr_in);

  SocketListNode *tmp = malloc(sizeof(SocketListNode));
  tmp->sockfd = ++_socket_fd;
  tmp->addr = malloc(sizeof(SocketListNode));
  memcpy(tmp->addr, address, sizeof(struct sockaddr_in));
  tmp->next = head;
  head = tmp;

  return tmp->sockfd;
}

ssize_t __wrap_read(int fildes, void *buf, size_t nbyte) {
  // printf("## in read ##\n");
  for (SocketListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->sockfd == fildes) {
      if (state != ESTABLISHED)
        return 0;
      int start_time = clock();
      while (1) {
        // printf("looping, receiver_ackno = %u, receiver_seqno = %u, offset = %d, nbyte = %d\n", receiver_ackno, receiver_seqno, read_offset, nbyte);
        // printf("timer = %f\n", (double)(clock() - start_time) / CLOCKS_PER_SEC);
        if (receiver_ackno - receiver_seqno - 1 >= read_offset + nbyte)
          break;
        if ((double)(clock() - start_time) / CLOCKS_PER_SEC > 8.0 * (init_state == LISTEN ? 1 : 2))
          break;
        // sleep(1);
      }
      
      int count = receiver_ackno - receiver_seqno - 1 - read_offset;
      if (count > nbyte)
        count = nbyte;
      memcpy(buf, receiver_buffer + read_offset, sizeof(uint8_t) * count);
      read_offset += count;
      return count;
    }
  }
  return __real_read(fildes, buf, nbyte);
}

ssize_t __wrap_write(int fildes , const void *buf , size_t nbyte) {
  // printf("__wrap_write: fildes = %d, len = %d\n", fildes, nbyte);
  for (SocketListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->sockfd == fildes) {
      if (state != ESTABLISHED)
        return -1;
      if (sendData(buf, nbyte, write_offset) == -1)
        return -1;
      write_offset += nbyte;
      return nbyte;// no failure
    }
  }
  return __real_write(fildes, buf, nbyte);
}

int __wrap_close(int fildes) {
  for (SocketListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->sockfd == fildes) {
      int result = connectionClose();
      if (result == -1)
        return -1;
      int start_time = clock();
      while (1) {
        if (state == CLOSED || state == LISTEN || state == TIME_WAIT)
          break;
        if ((double)(clock() - start_time) / CLOCKS_PER_SEC > 15.0)
          break;
      }
      if (state == TIME_WAIT)
        state = init_state;
      // printf("after close(), the final state is %d\n", state);
      if (state != CLOSED && state != LISTEN)
        return -1;
      
      tmp->sockfd = -1;
      return 0;
    }
  }
  return __real_close(fildes);
}

int __wrap_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  return -1;
}


void __listen__() { {}
  state = LISTEN;
}
