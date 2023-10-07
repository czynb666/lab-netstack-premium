#include <stdio.h>
#include <stdlib.h>

#include <pcap.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "config.h"

int main() {

  pcap_if_t *devs;
  char error_buf[PCAP_ERRBUF_SIZE] = {0};

  printf("pcap version = %s\n", pcap_lib_version());

  if (pcap_findalldevs(&devs, error_buf) == -1) {
    printf("%s\n", error_buf);
    exit(-1);
  }

  if (devs == NULL) {
    printf("No net devices can be found\n");
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

        // printf("addr: ");
        // for (int j = 0; j < 14; j++) {
        //   printf("%d ", t->addr->sa_data[j]);
        // }
        // printf(", mask: ");
        // for (int j = 0; j < 14; j++) {
        //   printf("%d ", t->netmask->sa_data[j]);
        // }
      }
//        printf("addr: %s, mask: %s\n", t->addr->sa_data, t->netmask->sa_data);
    }
  }



  return 0;
}