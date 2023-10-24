#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "ip.h"
#include "rip.h"

TrieNode *DVTrieRoot;

TrieNode *newNode(void) {
  TrieNode *ret = malloc(sizeof(TrieNode));

  ret->hops = 1 << 30;
  ret->attenuate_timer = 0;
  ret->fresh = 0;
  ret->deviceID = -1;

  ret->ch[0] = NULL;
  ret->ch[1] = NULL;
  return ret;
}

void initDVTrie(void) {
  DVTrieRoot = newNode();

  struct in_addr netmask;
  netmask.s_addr = 0x00ffffff;
  for (int i = 0; getDeviceName(i) != NULL; i++) {
    struct in_addr localIP = *getLocalIP(i);
    // insert self, hops = 0
    insertDVTrie(localIP, netmask, 0, i);

    struct in_addr nextHopIP;
    nextHopIP.s_addr = localIP.s_addr ^ 0x03000000;
    // insert adjacent next hop, hops = 1
    insertDVTrie(nextHopIP, netmask, 1, i);
  }
}

void insertDVTrie(struct in_addr ip, struct in_addr mask, int hops, int deviceID) {
  uint32_t rev_ip = ((ip.s_addr & 255) << 24) + 
                    (((ip.s_addr >> 8) & 255) << 16) + 
                    (((ip.s_addr >> 16) & 255) << 8) + 
                    ((ip.s_addr >> 24) & 255);

  uint32_t rev_mask = ((mask.s_addr & 255) << 24) + 
                      (((mask.s_addr >> 8) & 255) << 16) + 
                      (((mask.s_addr >> 16) & 255) << 8) + 
                      ((mask.s_addr >> 24) & 255);

  // printf("insertDVTrie: ip = %s, rev_ip = 0x%x\n", inet_ntoa(ip), rev_ip);
  // printf("insertDVTrie: mask = %s, rev_mask = 0x%x\n", inet_ntoa(mask), rev_mask);
  // printf("hops = %d, deviceID = %d\n", hops, deviceID);


  TrieNode *tmp = DVTrieRoot;
  for (int i = 31; i >= 0; i--) {
    if (~rev_mask >> i & 1)
      break;
    if (tmp->ch[rev_ip >> i & 1] == NULL)
      tmp->ch[rev_ip >> i & 1] = newNode();
    tmp = tmp->ch[rev_ip >> i & 1];
  }

  if (hops + 1 < tmp->hops || tmp->attenuate_timer >= 5) {
    tmp->ip = ip;
    tmp->mask = mask;
    tmp->hops = hops + 1;
    tmp->attenuate_timer = 0;
    tmp->deviceID = deviceID;
    tmp->fresh = 1;
  } else if (hops + 1 == tmp->hops) {
    tmp->attenuate_timer = 0;
  } else if (hops + 1 > tmp->hops && tmp->hops > 1) {
    // tmp->hops = 0 means this is local IP, or this is manually added(with highest priority)
    // tmp->hops = 1 means this is direct link, don't change
    tmp->attenuate_timer++;
  }
  // printf("inserted, tmp = %llx, deviceID = %d\n", tmp, tmp->deviceID);
}

int setRoutingTable(struct in_addr dest, struct in_addr mask, const char *device) {
  insertDVTrie(dest, mask, 0, findDevice(device));
  return 0;
}

void dfs(TrieNode *tmp, int *cnt) {
  if (tmp->deviceID != -1)
    (*cnt)++;
  if (tmp->ch[0] != NULL)
    dfs(tmp->ch[0], cnt);
  if (tmp->ch[1] != NULL)
    dfs(tmp->ch[1], cnt);
}

void dfs_2(TrieNode *tmp, uint8_t *payload, int *cnt) {
  // printf("tmp = %llx, deviceID = %d\n", tmp, tmp->deviceID);
  if (tmp->deviceID != -1) {
    payload[*cnt * 12 + 0] = tmp->ip.s_addr & 255;
    payload[*cnt * 12 + 1] = tmp->ip.s_addr >> 8 & 255;
    payload[*cnt * 12 + 2] = tmp->ip.s_addr >> 16 & 255;
    payload[*cnt * 12 + 3] = tmp->ip.s_addr >> 24 & 255;

    payload[*cnt * 12 + 4] = tmp->mask.s_addr & 255;
    payload[*cnt * 12 + 5] = tmp->mask.s_addr >> 8 & 255;
    payload[*cnt * 12 + 6] = tmp->mask.s_addr >> 16 & 255;
    payload[*cnt * 12 + 7] = tmp->mask.s_addr >> 24 & 255;

    payload[*cnt * 12 + 8] = tmp->hops & 255;
    payload[*cnt * 12 + 9] = tmp->hops >> 8 & 255;
    payload[*cnt * 12 + 10] = tmp->hops >> 16 & 255;
    payload[*cnt * 12 + 11] = tmp->hops >> 24 & 255;
  
    (*cnt)++;
  }
  if (tmp->ch[0] != NULL)
    dfs_2(tmp->ch[0], payload, cnt);
  if (tmp->ch[1] != NULL)
    dfs_2(tmp->ch[1], payload, cnt);
}

void dfs_3(TrieNode *tmp) {
  if (tmp->fresh)
    tmp->fresh = 0;
  if (tmp->ch[0] != NULL)
    dfs_3(tmp->ch[0]);
  if (tmp->ch[1] != NULL)
    dfs_3(tmp->ch[1]);
}

// find all fresh info and send them
void sendRIPpacket(void) {
  int cnt = 0;
  dfs(DVTrieRoot, &cnt);
  uint8_t *payload = malloc(sizeof(uint8_t) * cnt * 12);
  cnt = 0;
  dfs_2(DVTrieRoot, payload, &cnt);

  // printf("---------------\n");
  // printf("RIP: cnt = %d\n", cnt);
  // printf("---------------\n");

  if (cnt == 0)
    return;

  int allSuccess = 1;
  for (int i = 0; getDeviceName(i) != NULL; i++) {
    struct in_addr src = *getLocalIP(i);
    struct in_addr dest;
    dest.s_addr = src.s_addr ^ 0x03000000;

    if (sendIPPacket(src, dest, 0xFE, payload, cnt * 12, 64) == -1)
      allSuccess = 0;
  }
  // printf("allSuccess = %d\n", allSuccess);
  // if (allSuccess)
  //   dfs_3(DVTrieRoot);
}

void processRIPpacket(const uint8_t *packet, int payloadLength, int deviceID) {
  // printf("---------------------------\n");
  // printf("processing:\n");
  // for (int i = 0; i < payloadLength; i++) {
  //   printf("%02x ", packet[i]);
  // }
  // printf("\n---------------------------\n");
  
  for (int i = 0; i < payloadLength / 12; i++) {
    struct in_addr ip;
    struct in_addr mask;
    uint32_t hops;
    memcpy(&ip.s_addr, packet + i * 12, sizeof(uint32_t));
    memcpy(&mask.s_addr, packet + i * 12 + 4, sizeof(uint32_t));
    memcpy(&hops, packet + i * 12 + 8, sizeof(uint32_t));
    insertDVTrie(ip, mask, hops, deviceID);
  }
}

int route(const struct in_addr ip) {
  uint32_t rev_ip = ((ip.s_addr & 255) << 24) + 
                    (((ip.s_addr >> 8) & 255) << 16) + 
                    (((ip.s_addr >> 16) & 255) << 8) + 
                    ((ip.s_addr >> 24) & 255);
  // printf("routing, ip = %s, rev_ip = 0x%x\n", inet_ntoa(ip), rev_ip);
  TrieNode *tmp = DVTrieRoot;
  for (int i = 31; i >= 0; i--) {
    if (tmp->ch[rev_ip >> i & 1] == NULL)
      break;
    tmp = tmp->ch[rev_ip >> i & 1];
  }
  return tmp->deviceID;
}

TrieNode *getDVTrieRoot(void) {
  return DVTrieRoot;
}