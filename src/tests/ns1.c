#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <pcap.h>

#include "ethernet/device.h"
#include "ethernet/packetio.h"

#include "ip/ip.h"
#include "ip/arp.h"
#include "ip/rip.h"

void *packet_capture(void *arg) {
  int i = *(int *)arg;
  pcap_loop(getDeviceHandle(i), -1, &processFrame, getDeviceName(i));
}

void *RIP_send(void *arg) {
  while (1) {
    sendRIPpacket();
    sleep(5);
  }
}

int main() {

  char error_buf[PCAP_BUF_SIZE] = {0};
  if (pcap_init(PCAP_CHAR_ENC_LOCAL, error_buf) != 0) {
    printf("%s\n", error_buf);
    exit(-1);
  }


  showLocalInterfaces();
  addLocalInterfaces();
  initDVTrie();

  pthread_t capture_thread[5];
  int args[5];
  
  for (int i = 0; getDeviceName(i) != NULL; i++) {
    args[i] = i;
    if (pthread_create(&capture_thread[i], NULL, packet_capture, &args[i]) != 0) {
      exit(-1);
    }
  }

  pthread_t RIP_thread;

  if (pthread_create(&RIP_thread, NULL, RIP_send, NULL) != 0) {
    exit(-1);
  }

  struct in_addr target_ip;
  target_ip.s_addr = 0x0203640a;
  
  char s[] = "Hello World!";

  while (1) {
    sendIPPacket(*getLocalIP(0), target_ip, 0xFD, s, 12, 64);  
    sleep(7);
  }


  return 0;
}