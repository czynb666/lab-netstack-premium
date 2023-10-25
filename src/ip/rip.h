/**
 * @file rip.h
 * @brief implement RIP and manage routing table.
*/

#ifndef _IP_RIP_H
#define _IP_RIP_H


typedef struct TrieNode {
  
  struct in_addr ip;
  struct in_addr mask;

  int hops;
  int attenuate_timer;

  int deviceID;

  struct TrieNode *ch[2];

} TrieNode;

void initDVTrie(void);
void insertDVTrie(struct in_addr ip, struct in_addr mask, int hops, int deviceID);

void sendRIPpacket(void);

int setRoutingTable(struct in_addr dest, struct in_addr mask, const char *device);

void processRIPpacket(const uint8_t *packet, int payloadLength, int deviceID);

int route(const struct in_addr ip);

TrieNode *getDVTrieRoot(void);

#endif