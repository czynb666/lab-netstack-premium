#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>

#include <pcap.h>

#include <ip/ip.h>

#include "device.h"
#include "packetio.h"


/**
* @brief Encapsulate some data into an Ethernet II frame and send it.
*
* @param buf Pointer to the payload.
* @param len Length of the payload.
* @param ethtype EtherType field value of this frame.
* @param destmac MAC address of the destination.
* @param id ID of the device(returned by ‘addDevice ‘) to send on.
* @return 0 on success , -1 on error.
* @see addDevice
*/
int sendFrame(const void* buf, int len, uint16_t ethtype, const void* destmac, int id) {

  pcap_t *handle = getDeviceHandle(id);
  if (handle == NULL) {
    return -1;
  }

  int total_length = len + 14;
  if (total_length > 1500) {
    return -1;
  }

  if (total_length < 60)
    total_length = 60;

  uint8_t *frame = malloc(sizeof(uint8_t) * total_length);


  // destmac, sourcemac, ethtype, payload
  memcpy(frame, destmac, sizeof(uint8_t) * 6);

  uint8_t* source_mac_addr = getLocalMAC(id);
  memcpy(frame + 6, source_mac_addr, sizeof(uint8_t) * 6);

  ethtype = htons(ethtype);
  memcpy(frame + 12, &ethtype, sizeof(uint16_t));
  memcpy(frame + 14, buf, sizeof(uint8_t) * len);

  // printf("sending: total_length = %d\n", total_length);
  // for (int i = 0; i < total_length; i++) {
  //   printf("0x%x ", frame[i]);
  // }
  // puts("");
  if (pcap_sendpacket(handle, frame, total_length) != 0) {
    pcap_perror(handle, 0);
    return -1;
  }


  return 0;
}


/**
 * A callback function to be used by pcap_loop() or pcap_next().
 * 
 * @param deviceName the name of the device that receives a frame.
 * @param pkthdr the packet header of the frame.
 * @param packet the pointer to the packet.
*/
void printFrameInfo(unsigned char *deviceName, const struct pcap_pkthdr *pkthdr, const unsigned char *packet) {
  printf("%s receives a frame.\n", deviceName);

  printf("dest mac: %02x:%02x:%02x:%02x:%02x:%02x\n", packet[0], packet[1], packet[2], 
                                                        packet[3], packet[4], packet[5]);
  printf("source mac: %02x:%02x:%02x:%02x:%02x:%02x\n", packet[6], packet[7], packet[8], 
                                                      packet[9], packet[10], packet[11]);

  printf("\n");                                                    
}

void processFrame(unsigned char *deviceName, const struct pcap_pkthdr *pkthdr, const unsigned char *packet) {
  uint16_t protocol = (packet[12] << 8) + packet[13];
  switch (protocol) {
    case 0x0800:
      processIPpacket(packet + 14, findDevice(deviceName));
      break;
    case 0x0806:
      processARPpacket(packet + 14, findDevice(deviceName));
      break;
    default:
      break;
  }
}