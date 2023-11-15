/**
 * @file connection.h
 * @brief the library that maintain different connections
*/

#ifndef _TCP_CONNECTION_H
#define _TCP_CONNECTION_H

/* tcp states */
enum STATE {
  CLOSED,
  LISTEN,
  SYN_RCVD,
  SYN_SENT,
  ESTABLISHED,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSING,
  TIME_WAIT,
  CLOSE_WAIT,
  LAST_ACK,
};

void connectionInit();

int activeEstablish(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port);

int connectionClose();

void receiveSYN(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port, uint32_t _seqno);

void receiveSYNACK(struct in_addr _dst, struct in_addr _src, uint16_t _dst_port, uint16_t _src_port, uint32_t _seqno, uint32_t _ackno);

void receiveACK(uint32_t _ackno);

void receiveFIN();

#endif