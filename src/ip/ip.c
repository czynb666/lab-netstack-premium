#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "rip.h"
#include "arp.h"
#include "ip.h"

int sendIPPacket(const struct in_addr src, const struct in_addr dest, int proto, const void *buf, int len, int TTL) {
  
  if (TTL == 0)
    return -1;

  // printf("send IP packet: src = %s, ", inet_ntoa(src));
  // printf("dest = %s, ", inet_ntoa(dest));
  // printf("proto = 0x%x, len = %d, TTL = %d\n", proto, len, TTL);
  
  int deviceID = route(dest);
  if (deviceID == -1)
    return -1;


  struct in_addr deviceIP = *getLocalIP(deviceID);
  struct in_addr nextHopDeviceIP;
  nextHopDeviceIP.s_addr = deviceIP.s_addr ^ 0x03000000;


  // printf("routed, deviceID = %d\n", deviceID);
  // printf("deviceIP = %s\n", inet_ntoa(deviceIP));
  // printf("nextHopDeviceIP = %s\n", inet_ntoa(nextHopDeviceIP));

  uint8_t nextHopMAC[6];
  if (getMACaddress(&nextHopDeviceIP, nextHopMAC, -1) == -1)
    return -1;
  
  // assemble an IP header
  uint8_t *packet = malloc(sizeof(uint8_t) * (20 + len));

  // IP version = 4, header length = 20. binary = 0100 0101
  packet[0] = 0x45;

  // TOS = 0
  packet[1] = 0;

  // total length
  packet[2] = (20 + len) >> 8 & 255;
  packet[3] = (20 + len) & 255;

  // identifier = 0 (no fragmentation)
  packet[4] = 0;
  packet[5] = 0;

  // flags and fragments offset = 0
  packet[6] = 0;
  packet[7] = 0;

  // TTL
  packet[8] = TTL;

  // protocol
  packet[9] = proto;

  // checksum = 0
  packet[10] = 0;
  packet[11] = 0;

  // source IP
  packet[12] = src.s_addr & 255;
  packet[13] = src.s_addr >> 8 & 255;
  packet[14] = src.s_addr >> 16 & 255;
  packet[15] = src.s_addr >> 24 & 255;

  // dest IP
  packet[16] = dest.s_addr & 255;
  packet[17] = dest.s_addr >> 8 & 255;
  packet[18] = dest.s_addr >> 16 & 255;
  packet[19] = dest.s_addr >> 24 & 255;

  // payload
  memcpy(packet + 20, buf, sizeof(uint8_t) * len);

  sendFrame(packet, 20 + len, 0x0800, nextHopMAC, deviceID);

  return 0;
}


void processARPpacket(const uint8_t *packet, int last_id) {
  uint16_t opcode = ((uint16_t)packet[6] << 8) + packet[7];
  if (opcode == 0x0001) {// request
    processARPrequest(packet, last_id);
  }
  if (opcode == 0x0002) {// reply
    processARPreply(packet);
  }
}

void processIPpacket(const uint8_t *packet, int deviceID) {
  int headerLength = (packet[0] & 15) * 4;
  int payloadLength = (packet[2] << 8) + packet[3] - headerLength;
  uint8_t protocol = packet[9];
  if (protocol == 0xFE) {// it is set on purpose, to make RIP not based on UDP.
    processRIPpacket(packet + headerLength, payloadLength, deviceID);
  }
  if (protocol == 0xFD) {// our own IP packets.
    struct in_addr src_addr;
    src_addr.s_addr = (packet[15] << 24) + (packet[14] << 16) + (packet[13] << 8) + packet[12];

    struct in_addr dest_addr;
    dest_addr.s_addr = (packet[19] << 24) + (packet[18] << 16) + (packet[17] << 8) + packet[16];

    printf("src = %s\n", inet_ntoa(src_addr));
    printf("dest = %s\n", inet_ntoa(dest_addr));

    int TTL = packet[8];

    // check if this is the dest
    for (int i = 0; getDeviceName(i) != NULL; i++) {
      if (getLocalIP(i)->s_addr == dest_addr.s_addr) {
        printf("packet arrived, TTL = %d\n", TTL);
        return;
      }
    }

    // otherwise try to forward it
    // ECN or something else can be applied here I guess
    printf("forward packet, TTL = %d\n", TTL);
    sendIPPacket(src_addr, dest_addr, protocol, packet + headerLength, payloadLength, TTL - 1);

  }
}