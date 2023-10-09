### Writing Task 1

$\bf 1.$

$729$

$\bf 2.$

The dst is broadcast ( $\texttt{ff.ff.ff.ff}$ ). This frame is a DHCP discover frame.

$\bf 3.$

$15$



### Programming Task 1

I implemented these function in `device.h, device.c`

```c
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
```

### Programming Task 2

I implemented these function in `packetio.h, packetio.c`

``` c
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
```

