#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#include "device.h"

#define SNAPLEN 1024
#define TIMEOUT 1000

typedef struct ListNode {
  int id;
  char *name;
  pcap_t *handle;
  struct ListNode *next;
} ListNode;

int total_id;
ListNode *head;

/**
* Add a device to the library for sending/receiving packets.
*
* @param device Name of network device to send/receive packet on.
* @return A non-negative _device-ID_ on success , -1 on error.
*/
int addDevice(const char* device) {

  char error_buf[PCAP_ERRBUF_SIZE] = {0};

  pcap_t *handle = pcap_open_live(device, SNAPLEN, 1, TIMEOUT, error_buf);

  if (handle == NULL) {
    printf("%s\n", error_buf);
    return -1;
  }

  ListNode *tmp = malloc(sizeof(ListNode));
  
  tmp->id = total_id++;
  tmp->name = malloc(strlen(device) * sizeof(char));
  strcpy(tmp->name, device);
  tmp->handle = handle;
  tmp->next = head;
  head = tmp;

  return tmp->id;

}

/**
* Find a device added by ‘addDevice ‘.
*
* @param device Name of the network device.
* @return A non-negative _device-ID_ on success , -1 if no such device
* was found.
*/
int findDevice(const char* device) {
  for (ListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (!strcmp(tmp->name, device))
      return tmp->id;
  }
  return -1;
}

/**
 * Get the name of a device by id.
 * 
 * @param id the id to look up.
 * @return the name on success, NULL if fails.
*/
char *getDeviceName(int id) {
  for (ListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->id == id)
      return tmp->name;
  }
  return NULL;
}

/**
 * Get the handle of a device by id.
 * 
 * @param id the id to look up.
 * @return the pcap_t handle on success, NULL if fails.
*/
pcap_t *getDeviceHandle(int id) {
  for (ListNode *tmp = head; tmp != NULL; tmp = tmp->next) {
    if (tmp->id == id)
      return tmp->handle;
  }
  return NULL;
}

/**
 * Detect local interfaces and print them.
*/
void showLocalInterfaces(void) {
  pcap_if_t *devs;
  char error_buf[PCAP_ERRBUF_SIZE] = {0};

  printf("pcap version = %s\n", pcap_lib_version());

  if (pcap_findalldevs(&devs, error_buf) == -1) {
    printf("%s\n", error_buf);
    exit(-1);
  }

  if (devs == NULL) {
    printf("No net interfaces can be found\n");
    exit(-1);
  }


  pcap_if_t *tmp = devs;
  for (int i = 0; tmp != NULL; i++, tmp = tmp->next) {
    printf("devices: %d, %s, %s\n", i, tmp->name, tmp->description);
    for (struct pcap_addr *t = tmp->addresses; t != NULL; t = t->next) {
      if (t->addr != NULL && t->netmask != NULL) {

        struct sockaddr_in *p_addr = (struct sockaddr_in*) t->addr;
        struct sockaddr_in *p_mask = (struct sockaddr_in*) t->netmask;

        printf("addr: %s, mask: %s\n", inet_ntoa(p_addr->sin_addr), inet_ntoa(p_mask->sin_addr));

      }
    }
  }

}