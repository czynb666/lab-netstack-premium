#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "arp.h"

typedef struct ListNode {
  struct in_addr *target;
  uint8_t *mac_address;
  struct ListNode *next;
} ListNode;

ListNode *ARPcacheHead;

void insertARPcache(struct in_addr *target, uint8_t *mac_address) {

  // printf("@@ inserting, target = %s, ", inet_ntoa(*target));
  // for (int i = 0; i < 6; i++) {
  //   printf("%02x ", mac_address[i]);
  // }
  // puts("");
  ListNode *tmp = malloc(sizeof(ListNode));
  tmp->target = malloc(sizeof(struct in_addr));
  memcpy(tmp->target, target, sizeof(struct in_addr));
  tmp->mac_address = malloc(sizeof(uint8_t) * 6);
  memcpy(tmp->mac_address, mac_address, sizeof(uint8_t) * 6);
  tmp->next = ARPcacheHead;

  ARPcacheHead = tmp;
}

int findARPcache(struct in_addr *target_ip, uint8_t *mac_address) {
  for (ListNode *tmp = ARPcacheHead; tmp != NULL; tmp = tmp->next) {
    if (target_ip->s_addr == tmp->target->s_addr) {
      memcpy(mac_address, tmp->mac_address, sizeof(uint8_t) * 6);
      return 0;
    }
  }
  return -1;
}


/**
 * @brief get the MAC address corresponded to the IP address
 * @param target the IP address
 * @param last_id the id of the device which receives this ARP request. -1 if none.
 * @return -1 on failure. 0 on success, and setting mac_address.
*/
int getMACaddress(struct in_addr *target_ip, uint8_t *mac_address, int last_id) {

  if (findARPcache(target_ip, mac_address) == 0)
    return 0;

  sendARPrequest(target_ip, last_id);// broadcast

  return -1;
}

/**
 * @brief send an ARP broadcast
*/
void sendARPrequest(struct in_addr *target_ip, int last_id) {
  // assemble an ARP content
  uint8_t *ARPcontent = malloc(sizeof(uint8_t) * 28);

  // hardware type
  ARPcontent[0] = 0x00, ARPcontent[1] = 0x01;
  
  // protocol type
  ARPcontent[2] = 0x08, ARPcontent[3] = 0x00;
  
  // hardware size and protocol size
  ARPcontent[4] = 0x06, ARPcontent[5] = 0x04;

  // opt code: request
  ARPcontent[6] = 0x00, ARPcontent[7] = 0x01;

  // find all possible devices
  for (int i = 0; getDeviceName(i) != NULL; i++) {
    
    // don't send back
    if (i == last_id)
      continue;

    // source mac address
    uint8_t *source_mac_addr = getLocalMAC(i);
    memcpy(ARPcontent + 8, source_mac_addr, sizeof(uint8_t) * 6);

    // source ip address, big endian to small endian
    struct in_addr *source_ip = getLocalIP(i);
    ARPcontent[14] = source_ip->s_addr & 255;
    ARPcontent[15] = source_ip->s_addr >> 8 & 255;
    ARPcontent[16] = source_ip->s_addr >> 16 & 255;
    ARPcontent[17] = source_ip->s_addr >> 24 & 255;
    
    // dest mac address = 0
    memset(ARPcontent + 18, 0, sizeof(uint8_t) * 6);

    // dest ip address, big endian to small endian
    ARPcontent[24] = target_ip->s_addr & 255;
    ARPcontent[25] = target_ip->s_addr >> 8 & 255;
    ARPcontent[26] = target_ip->s_addr >> 16 & 255;
    ARPcontent[27] = target_ip->s_addr >> 24 & 255;
    
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    // printf("i = %d, sending...\n", i);
    // printf("i = %d, send_res = %d\n", sendFrame(ARPcontent, 28, 0x0806, broadcast_mac, i));
    sendFrame(ARPcontent, 28, 0x0806, broadcast_mac, i);
  }
}

void sendARPreply(struct in_addr *dest_ip, uint8_t *dest_mac_address, uint8_t *source_mac_address, int last_id) {
  // assemble an ARP content
  uint8_t *ARPcontent = malloc(sizeof(uint8_t) * 28);

  // hardware type
  ARPcontent[0] = 0x00, ARPcontent[1] = 0x01;
  
  // protocol type
  ARPcontent[2] = 0x08, ARPcontent[3] = 0x00;
  
  // hardware size and protocol size
  ARPcontent[4] = 0x06, ARPcontent[5] = 0x04;

  // opt code: reply
  ARPcontent[6] = 0x00, ARPcontent[7] = 0x02;

  // source mac address
  memcpy(ARPcontent + 8, source_mac_address, sizeof(uint8_t) * 6);

  // source ip address, big endian to small endian
  struct in_addr *source_ip = getLocalIP(last_id);
  ARPcontent[14] = source_ip->s_addr & 255;
  ARPcontent[15] = source_ip->s_addr >> 8 & 255;
  ARPcontent[16] = source_ip->s_addr >> 16 & 255;
  ARPcontent[17] = source_ip->s_addr >> 24 & 255;
  
  // dest mac address
  memcpy(ARPcontent + 18, dest_mac_address, sizeof(uint8_t) * 6);

  // dest ip address, big endian to small endian
  ARPcontent[24] = dest_ip->s_addr & 255;
  ARPcontent[25] = dest_ip->s_addr >> 8 & 255;
  ARPcontent[26] = dest_ip->s_addr >> 16 & 255;
  ARPcontent[27] = dest_ip->s_addr >> 24 & 255;

  // printf("i = %d, sending...\n", i);
  // printf("send_res = %d\n", sendFrame(ARPcontent, 28, 0x0806, dest_mac_address, last_id));
  sendFrame(ARPcontent, 28, 0x0806, dest_mac_address, last_id);
}

void processARPrequest(const uint8_t *packet, int last_id) {

  // dest is the source of {packet}; source is target of {packet}, a.k.a. local

  uint32_t dest_s_addr = (packet[17] << 24) + (packet[16] << 16) + 
                         (packet[15] << 8) + packet[14];
  uint32_t target_s_addr = (packet[27] << 24) + (packet[26] << 16) + 
                           (packet[25] << 8) + packet[24];
  struct in_addr dest_addr;
  struct in_addr target_addr;
  dest_addr.s_addr = dest_s_addr;
  target_addr.s_addr = target_s_addr;
  
  // in the reply packet, target mac address = source mac address
  uint8_t *target_mac_address = malloc(sizeof(uint8_t) * 6);
  // in the reply packet, dest mac address = dest mac address
  uint8_t *dest_mac_address = malloc(sizeof(uint8_t) * 6);
  memcpy(dest_mac_address, packet + 8, sizeof(uint8_t) * 6);

  
  // printf("processing ARP request: dest_addr = %s, target_addr = %s\n", inet_ntoa(dest_addr), inet_ntoa(target_addr));
  // for (int i = 0; i < 28; i++) {
  //   printf("%02x ", packet[i]);
  // }
  // puts("");

  // find ARP cache
  if (findARPcache(&target_addr, target_mac_address) == 0) {
    sendARPreply(&dest_addr, dest_mac_address, target_mac_address, last_id);
    return;
  }

  // find local devices
  for (int i = 0; getDeviceName(i) != NULL; i++) {
    if (getLocalIP(i)->s_addr == target_s_addr) {
      memcpy(target_mac_address, getLocalMAC(i), sizeof(uint8_t) * 6);
      insertARPcache(&target_addr, target_mac_address);
      sendARPreply(&dest_addr, dest_mac_address, target_mac_address, last_id);
      break;
    }
  }

  // assume just one hop?
  // sendARPrequest(&target_addr, last_id); 

}

void processARPreply(const uint8_t *packet) {
  // assume just one hop, so this arrived.
  
  uint32_t target_s_addr = (packet[17] << 24) + (packet[16] << 16) + 
                           (packet[15] << 8) + packet[14];

  struct in_addr target_addr;
  target_addr.s_addr = target_s_addr;

  // printf("#############\n");
  // printf("received ARP, target = %s\n", inet_ntoa(target_addr));
  // printf("#############\n");
  
  uint8_t target_mac_address[6];
  memcpy(target_mac_address, packet + 8, sizeof(uint8_t) * 6);
  insertARPcache(&target_addr, target_mac_address);

}