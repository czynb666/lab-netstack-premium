/**
 * @file arp.h
 * @brief implement ARP.
*/

#ifndef _IP_ARP_H
#define _IP_ARP_H

#include <netinet/in.h>

/**
 * @brief get the MAC address corresponded to the IP address
 * @param target the IP address
 * @param last_id the id of the device which receives this ARP request. -1 if none.
 * @return -1 on failure. 0 on success, and setting mac_address.
*/
int getMACaddress(struct in_addr *target_ip, uint8_t *mac_address, int last_id);

void sendARPrequest(struct in_addr *target_ip, int last_id);

void processARPrequest(const uint8_t *packet, int last_id);
void processARPreply(const uint8_t *packet);

#endif