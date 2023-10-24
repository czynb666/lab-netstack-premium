#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pcap.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"


int main() {

  char error_buf[PCAP_BUF_SIZE] = {0};
  if (pcap_init(PCAP_CHAR_ENC_LOCAL, error_buf) != 0) {
    printf("%s\n", error_buf);
    exit(-1);
  }
  
  // int id = addDevice("eth0");
  // printf("add eth0: %d\n", id);


  // char str[] = "HelloWorld!";
  // uint8_t destmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  // for (int i = 0; i < 5; i++) {
  //   printf("send frame: %d\n", sendFrame(str, strlen(str), 0x0800, destmac, id));
  // }

  return 0;
}