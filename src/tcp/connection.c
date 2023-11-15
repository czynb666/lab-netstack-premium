#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "tcp.h"
#include "connection.h"

/*
typedef struct TCPListNode {
  struct in_addr dst;
  struct in_addr src;
  uint16_t dst_port;
  uint16_t src_port;

  enum STATE state;
  int *receivedACK;// length = w
  uint32_t seqno;

  struct TCPListNode *next;
} TCPListNode;
TCPListNode *TCPListHead;
*/

struct in_addr dst;
struct in_addr src;
uint16_t dst_port;
uint16_t src_port;

enum STATE state;

extern uint32_t sender_seqno;
extern uint32_t sender_ackno;
extern uint32_t receiver_seqno;
extern uint32_t receiver_ackno;
extern int sender_receivedACK[BUFFER_SIZE];
extern int receiver_receivedData[BUFFER_SIZE];
extern int read_offset;
extern int write_offset;

void clearBuffer() {
  memset(sender_receivedACK, 0, sizeof(sender_receivedACK));
  memset(receiver_receivedData, 0, sizeof(receiver_receivedData));
  read_offset = 0;
  write_offset = 0;
}

void connectionInit() {
  srand(time(0));
  state = CLOSED;
}

int activeEstablish(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port) {
  if (state != CLOSED) {
    // printf("active establish: state != CLOSED\n");
    return -1;
  }
  // printf("## activeEstablish: dst_port = %d, src_port = %d ##\n", _dst_port, _src_port);
  dst = _dst;
  src = _src;
  dst_port = _dst_port;
  src_port = _src_port;
  sender_seqno = (rand() << 16) + rand();
  sender_ackno = sender_seqno;
  if (sendSegment(SYN, NULL, 0, 0) == 0) {
    state = SYN_SENT;
    sleep(10);
    // printf("## segment sent, state = %d ##\n", state);
    if (state == ESTABLISHED) {
      clearBuffer();
      return 0;
    }
    else {
      state = CLOSED;
      sendSegment(RST, NULL, 0, 0);
      return -1;
    }
  }
  // printf("## sendSegment failed.. ##\n");
  return -1;
}

int connectionClose() {
  switch (state) {
    case LISTEN:
      state = CLOSED;
      return 0;
    case SYN_SENT:
      state = CLOSED;
      return 0;
    case SYN_RCVD:
      state = FIN_WAIT_1;
      sendSegment(FIN, NULL, 0, 0);
      return 0;
    case ESTABLISHED:
      state = FIN_WAIT_1;
      sendSegment(FIN, NULL, 0, 0);
      return 0;
    case CLOSE_WAIT:
      state = LAST_ACK;
      sendSegment(FIN, NULL, 0, 0);
      return 0;
    default:
      printf("connectionClose: state error, state = %d\n", state);
      exit(-1);
      return -1;
  }
}

void receiveSYN(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port, uint32_t _seqno) {
  dst = _dst;
  src = _src;
  dst_port = _dst_port;
  src_port = _src_port;
  switch (state) {
    case LISTEN:
      clearBuffer();
      receiver_seqno = _seqno;
      receiver_ackno = _seqno + 1;
      sender_seqno = (rand() << 16) + rand();
      sender_ackno = sender_seqno;
      sendSegment(SYN | ACK, NULL, 0, 0);
      state = SYN_RCVD;
      break;
    case SYN_SENT:
      sendSegment(SYN | ACK, NULL, 0, 0);
      state = SYN_RCVD;
      break;
    default:
      printf("receive SYN: state error\n");
      // exit(-1);
      break;
  }
}

void receiveSYNACK(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port, uint32_t _seqno, uint32_t _ackno) {
  switch (state) {
    case SYN_SENT:
      sender_ackno = _ackno;
      if (sender_ackno != sender_seqno + 1) {
        printf("something is wrong\n");
        exit(-1);
      }
      receiver_seqno = _seqno;
      receiver_ackno = _seqno + 1;
      sendSegment(ACK, NULL, 0, 0);
      state = ESTABLISHED;
      break;
    default:
      printf("receive SYNACK: state error\n");
      // exit(-1);
      break;
  }
}

void receiveACK(uint32_t _ackno) {
  if (_ackno > sender_ackno)
    sender_ackno = _ackno;
  switch (state) {
    case SYN_RCVD:
      state = ESTABLISHED;
      break;
    case ESTABLISHED:
      processDataACK(_ackno);
      break;
    case FIN_WAIT_1:
      state = FIN_WAIT_2;
      break;
    case CLOSING:
      state = TIME_WAIT;
      break;
    case LAST_ACK:
      state = LISTEN;
      break;
    default:
      printf("receive ACK: state error\n");
      // exit(-1);
      break;
  }
}

void receiveFIN() {
  switch (state) {
    case ESTABLISHED:
      state = CLOSE_WAIT;
      sendSegment(ACK, NULL, 0, 0);
      break;
    case FIN_WAIT_1:
      sendSegment(ACK, NULL, 0, 0);
      state = CLOSING;
      break;
    case FIN_WAIT_2:
      sendSegment(ACK, NULL, 0, 0);
      state = TIME_WAIT;
      break;
    default:
      printf("receive FIN: state error\n");
      // exit(-1);
      break;
  }
}

void receiveRST() {
  switch (state) {
    case SYN_RCVD:
      printf("connection reset\n");
      state = LISTEN;
      break;
    default:
      printf("receive RST: state error\n");
      break;
  }
}