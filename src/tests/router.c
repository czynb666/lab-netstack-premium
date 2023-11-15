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

#include "tcp/tcp.h"
#include "tcp/connection.h"

int main() {

  setupNetStack();
  while (1) {}

  return 0;
}