/**
* @file device.h
* @brief Library supporting network device management.
*/

#ifndef _ETHERNET_DEVICE_H
#define _ETHERNET_DEVICE_H

#include <netinet/in.h>
#include <pcap.h>

/**
* Add a device to the library for sending/receiving packets.
*
* @param device Name of network device to send/receive packet on.
* @return A non-negative _device-ID_ on success , -1 on error.
*/
int addDevice(const char* device, struct in_addr *ip, struct in_addr *mask);

/**
* Find a device added by ‘addDevice ‘.
*
* @param device Name of the network device.
* @return A non-negative _device-ID_ on success , -1 if no such device
* was found.
*/
int findDevice(const char* device);

/**
 * Get the handle of a device by id.
 * 
 * @param id the id to look up.
 * @return the pcap_t handle on success, NULL if fails.
*/
pcap_t *getDeviceHandle(int id);

/**
 * Get the name of a device by id.
 * 
 * @param id the id to look up.
 * @return the name on success, NULL if fails.
*/
char *getDeviceName(int id);

/**
 * Detect local interfaces and print them.
*/
void showLocalInterfaces(void);

void addLocalInterfaces(void);

uint8_t *getLocalMAC(int id);
struct in_addr *getLocalIP(int id);

#endif