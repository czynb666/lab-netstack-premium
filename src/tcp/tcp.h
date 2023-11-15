/**
 * @file tcp.h
 * @brief provide bidirectional, reliable, ordered transmission
*/

#ifndef _TCP_TCP_H
#define _TCP_TCP_H

#include <netinet/ip.h>


#define BUFFER_SIZE 2000000
#define MAX_PAYLOAD_LENGTH 1400
#define WINDOW_SIZE (MAX_PAYLOAD_LENGTH * 10)

enum {
  FIN = 0x01,
  SYN = 0x02,
  RST = 0x04,
  PSH = 0x08,
  ACK = 0x10,
  URG = 0x20,
};


void tcpInit();

int sendSegment(uint16_t command, void *data, int len, int offset);

int sendData(const uint8_t *data, int len, int offset);

void processTCPsegment(const uint8_t *segment, int len, struct in_addr seg_src, struct in_addr seg_dst);

void processDataACK(uint32_t _ackno);

void processData(const uint8_t *data, int len, uint32_t _seqno);

#endif 