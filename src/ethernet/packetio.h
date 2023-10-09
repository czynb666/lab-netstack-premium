/**
* @file packetio.h
* @brief Library supporting sending/receiving Ethernet II frames.
*/

#ifndef _ETHERNET_PACKETIO_H
#define _ETHERNET_PACKETIO_H

#include <netinet/ether.h>

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
int sendFrame(const void* buf , int len ,
uint16_t ethtype , const void* destmac , int id);

/**
 * A callback function to be used by pcap_loop() or pcap_next().
 * 
 * @param deviceName the name of the device that receives a frame.
 * @param pkthdr the packet header of the frame.
 * @param packet the pointer to the packet.
*/
void printFrameInfo(unsigned char *deviceName, const struct pcap_pkthdr *pkthdr, const unsigned char *packet);

#endif