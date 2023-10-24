/**
* @file ip.h
* @brief Library supporting sending/receiving IP packets encapsulated
in an Ethernet II frame.
*/

#ifndef _IP_IP_H
#define _IP_IP_H

#include <netinet/ip.h>

/**
* @brief Send an IP packet to specified host.
*
* @param src Source IP address.
* @param dest Destination IP address.
* @param proto Value of ‘protocol ‘ field in IP header.
* @param buf pointer to IP payload
* @param len Length of IP payload
* @return 0 on success , -1 on error.
*/
int sendIPPacket(const struct in_addr src , const struct in_addr dest ,
int proto , const void *buf , int len, int TTL);


/**
* @brief Process an IP packet upon receiving it.
*
* @param buf Pointer to the packet.
* @param len Length of the packet.
* @return 0 on success , -1 on error.
* @see addDevice
*/
typedef int (* IPPacketReceiveCallback)(const void* buf , int len);


/**
* @brief Register a callback function to be called each time an IP
packet was received.
*
* @param callback The callback function.
* @return 0 on success , -1 on error.
* @see IPPacketReceiveCallback
*/
int setIPPacketReceiveCallback(IPPacketReceiveCallback callback);


void processARPpacket(const uint8_t *packet, int last_id);
void processIPpacket(const uint8_t *packet, int deviceID);

#endif