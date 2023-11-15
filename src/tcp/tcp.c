#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include <netinet/in.h>

#include "ip/ip.h"

#include "connection.h"
#include "tcp.h"


extern struct in_addr dst;
extern struct in_addr src;
extern uint16_t dst_port;
extern uint16_t src_port;
extern enum STATE state;

// sender
uint32_t sender_seqno;
uint8_t sender_buffer[BUFFER_SIZE];

uint32_t sender_ackno;
int sender_receivedACK[BUFFER_SIZE];
sem_t sem_ACK;


// receiver
uint32_t receiver_seqno;
uint8_t receiver_buffer[BUFFER_SIZE];

uint32_t receiver_ackno;
int receiver_receivedData[BUFFER_SIZE];


void tcpInit() {
  sem_init(&sem_ACK, 0, 1);
}

int sendSegment(uint16_t command, void *data, int len, int offset) {
  uint8_t *TCPsegment = malloc(sizeof(uint8_t) * (20 + len));

  // src port and dst port
  TCPsegment[0] = src_port >> 8;
  TCPsegment[1] = src_port & 255;
  TCPsegment[2] = dst_port >> 8;
  TCPsegment[3] = dst_port & 255;

  // seqno and ackno
  TCPsegment[4] = (sender_ackno + offset) >> 24 & 255;
  TCPsegment[5] = (sender_ackno + offset) >> 16 & 255;
  TCPsegment[6] = (sender_ackno + offset) >> 8 & 255;
  TCPsegment[7] = (sender_ackno + offset) & 255;
  
  TCPsegment[8] = receiver_ackno >> 24 & 255;
  TCPsegment[9] = receiver_ackno >> 16 & 255;
  TCPsegment[10] = receiver_ackno >> 8 & 255;
  TCPsegment[11] = receiver_ackno & 255;

  // offset = 20, reserved = 0, command = command
  TCPsegment[12] = 0x50;
  TCPsegment[13] = command;

  // window = WINDOW_SIZE
  TCPsegment[14] = WINDOW_SIZE >> 8 & 255;
  TCPsegment[15] = WINDOW_SIZE & 255;

  // checksum and urgent pointer = 0
  TCPsegment[16] = 0;
  TCPsegment[17] = 0;
  TCPsegment[18] = 0;
  TCPsegment[19] = 0;

  memcpy(TCPsegment + 20, data, sizeof(uint8_t) * len);

  return sendIPPacket(src, dst, 0x6, TCPsegment, 20 + len, 64);

}

void processTCPsegment(const uint8_t *segment, int len, struct in_addr seg_src, struct in_addr seg_dst) {
  // src and dst are based on "this" view.
  // so seg_src becomes dst, seg_dst becomes src

  uint16_t seg_src_port = (segment[0] << 8) + segment[1];
  uint16_t seg_dst_port = (segment[2] << 8) + segment[3];
  // printf("process TCP: src_port = %d, dst_port = %d\n", seg_src_port, seg_dst_port);
  // printf("process TCP: src = %s\n", inet_ntoa(seg_src));
  // printf("process TCP: dst = %s\n", inet_ntoa(seg_dst));

  uint16_t flags = segment[13];
  int header_length = segment[12] / 4;
  int payload_length = len - header_length;
  const uint8_t *payload = segment + header_length;

  // printf("process TCP: flags = %d, header_length = %d, payload_length = %d\n", flags, header_length, payload_length);

  uint32_t _seqno = (segment[4] << 24) + (segment[5] << 16) + (segment[6] << 8) + segment[7];
  uint32_t _ackno = (segment[8] << 24) + (segment[9] << 16) + (segment[10] << 8) + segment[11];

  if (flags & SYN && flags & ACK) {
    receiveSYNACK(seg_src, seg_dst, seg_src_port, seg_dst_port, _seqno, _ackno);
  } else if (flags & SYN) {
    receiveSYN(seg_src, seg_dst, seg_src_port, seg_dst_port, _seqno);
  } else if (flags & FIN) {
    receiveFIN();
  } else if (flags & ACK) {
    receiveACK(_ackno);
  } else if (flags & RST) {
    receiveRST();
  } else {
    if (state != ESTABLISHED) {// bad design here
      state = LISTEN;
      return;
    }
    processData(segment + header_length, payload_length, _seqno);
  }

}

/**
 * @brief send data through multiple segments. this reliable
 * @note the src, dst, src_port, dst_port are fixed. the state is assured to be ESTABLISHED
*/
int sendData(const uint8_t *data, int len, int offset) {
  memcpy(sender_buffer + offset, data, sizeof(uint8_t) * len);
  memset(sender_receivedACK + offset, 0, sizeof(int) * len);

  // printf("sendData: sender_seqno = %u, sender_ackno = %u\n", sender_seqno, sender_ackno);
  // printf("sendData: data len = %d, offset = %d\n", len, offset);

  int continuous_stall_count = 0;

  while (sender_ackno < sender_seqno + 1 + offset + len) {
    sem_wait(&sem_ACK);// well, it's gonna be slow and congested

    // printf("looping, delt = %u\n", sender_ackno - sender_seqno - 1 - offset);

    for (int i = 0, j; i + sender_ackno - sender_seqno - 1 - offset < len && i < WINDOW_SIZE; i = j + 1) {
      j = i;
      if (sender_receivedACK[i + sender_ackno - sender_seqno - 1])
        continue;
      while ((j + 1) - i + 1 < MAX_PAYLOAD_LENGTH &&
             (j + 1) + sender_ackno - sender_seqno - 1 - offset < len &&
             !sender_receivedACK[(j + 1) + sender_ackno - sender_seqno - 1]) {
        j++;
      }
      // printf("i = %d, j = %d\n", i, j);
      // sender_seglen[i + sender_ackno - sender_seqno - 1] = j - i + 1;
      sendSegment(0, data + i + sender_ackno - sender_seqno - 1 - offset, j - i + 1, i);
    }

    int last_ackno = sender_ackno;
    sem_post(&sem_ACK);

    sleep(3);

    if (sender_ackno == last_ackno) {
      continuous_stall_count++;
    } else {
      continuous_stall_count = 0;
    }
    if (continuous_stall_count == 3)
      return -1;

  }
  return 0;

}

/**
 * @brief got an data's ACK, not a state's ACK
*/
void processDataACK(uint32_t _ackno) {
  sem_wait(&sem_ACK);
  // printf("processDataACK: _ackno = %u, sender_ackno = %u\n", _ackno, sender_ackno);
  if (_ackno > sender_ackno) {
    for (int i = sender_ackno; i < _ackno; i++) {
      // printf("index = %d\n", i - sender_seqno - 1);
      sender_receivedACK[i - sender_seqno - 1] = 1;
    }
    while (sender_receivedACK[sender_ackno - sender_seqno - 1]) {
      sender_ackno++;
    }
  }
  // printf("processDataACK over, sender_ackno = %u\n", sender_ackno);

  sem_post(&sem_ACK);
}

void processData(const uint8_t *data, int len, uint32_t _seqno) {
  sem_wait(&sem_ACK);

  // printf("processData: len = %d, _seqno = %u\n", len, _seqno);
  // printf("processData: receiver_seqno = %u, receiver_ackno = %u\n", receiver_seqno, receiver_ackno);


  for (int i = 0; i < len; i++) {
    receiver_buffer[i + _seqno - receiver_seqno - 1] = data[i];
    receiver_receivedData[i + _seqno - receiver_seqno - 1] = 1;
//    printf("index = %d\n", _seqno - rece)
  }

  while (receiver_receivedData[receiver_ackno - receiver_seqno - 1]) {
    receiver_ackno++;
  }

  sendSegment(ACK, NULL, 0, 0);

  sem_post(&sem_ACK);
}

// void handleACK(uint32_t seqno, ) {

// }