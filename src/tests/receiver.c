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

  showLocalInterfaces();

  int id = addDevice("eth0");
  printf("add eth0: %d\n", id);

  struct bpf_program bpf;
  char filter[] = "ether dst ff:ff:ff:ff:ff:ff";
  pcap_t *handle = getDeviceHandle(id);
  bpf_u_int32 ip = 0;

  if (pcap_compile(handle, &bpf, filter, 0, 0) == -1) {
    pcap_perror(handle, NULL);
    exit(-1);
  }

  if (pcap_setfilter(handle, &bpf) == -1) {
    pcap_perror(handle, NULL);
    exit(-1);
  }

  pcap_loop(getDeviceHandle(id), 10, &printFrameInfo, getDeviceName(id));
  
  return 0;
}