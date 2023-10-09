/**
* @file device.h
* @brief Library supporting network device management.
*/

#ifndef _ETHERNET_DEVICE_C
#define _ETHERNET_DEVICE_C

/**
* Add a device to the library for sending/receiving packets.
*
* @param device Name of network device to send/receive packet on.
* @return A non-negative _device-ID_ on success , -1 on error.
*/
int addDevice(const char* device);

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

char *getDeviceName(int id);

void ethernetDeviceInit(void);

void showLocalInterfaces(void);

#endif