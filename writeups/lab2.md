### Writing Task 2

$\bf 1.$

The source MAC address.

$\bf 2.$

$1674$ .

$\bf 3.$

IPv4: $20$ bytes. IPv6: $40$ bytes. 

### Run program and tests

First change the destination IP address in `src/tests/ns1.c` . In every test, `ns1` regularly sends packets to a fixed IP address.

Run `sudo make` .

`cd vnetUtils/examples`, `sudo bash ./makeVNet < test1.txt` or `test2.txt, test3.txt`.

The three test networks are:

```
// test1
1 - 2 - 3 - 4

// test2
1 - 2 - 3 - 4
    |   |
    5 - 6

// test3
1 - 2 - 3
|   |   |
4 - 5 - 6
```

Then open a terminal with every ns hosts.

````bash
cd vnetUtils/helper; 
sudo ./execNS ns* bash
cd ../../build
````

Then `sudo ./ns1` or `sudo ./router` . We can also `sudo ./ns1` in another `ns` to make it the packet sender.

### Programming Task 3

I implemented these functions in `arp.h/c`:

``` c
int getMACaddress(struct in_addr *target_ip, uint8_t *mac_address, int last_id);

void sendARPrequest(struct in_addr *target_ip, int last_id);
void sendARPreply(struct in_addr *dest_ip, uint8_t *dest_mac_address, uint8_t *source_mac_address, int last_id);

void processARPrequest(const uint8_t *packet, int last_id);
void processARPreply(const uint8_t *packet);
```

I implemented these functions in `ip.h/c`:

```c
int sendIPPacket(const struct in_addr src , const struct in_addr dest ,
int proto , const void *buf , int len, int TTL);

void processARPpacket(const uint8_t *packet, int last_id);
void processIPpacket(const uint8_t *packet, int deviceID);
```

I implemented these functions in `rip.h/c`:

``` c
void initDVTrie(void);
void insertDVTrie(struct in_addr ip, struct in_addr mask, int hops, int deviceID);
void sendRIPpacket(void);
int setRoutingTable(struct in_addr dest, struct in_addr mask, const char *device);
void processRIPpacket(const uint8_t *packet, int payloadLength, int deviceID);
int route(const struct in_addr ip);
TrieNode *getDVTrieRoot(void);
```



### Writing Task 3

I implemented the ARP. If the caller of `sendFrame()` doesn't know the MAC address corresponded to the IP address, it broadcasts ARP packets to adjacent hosts. 

Here's an example with kernel protocol on: (the wireshark monitors vnet2-1 on ns2)

```
ns1 - ns2 (10.100.1.0/24)

(the whole network is below, but only ns1 and ns2 are activated)
1 - 2 - 3 - 4
    |   |
    5 - 6
```

![image-20231025195239055](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025195239055.png)

With kernel protocol on, the ARP request receives 2 ARP replies.

After receiving an ARP reply, the host adds the info to its ARP cache.

### Writing Task 4

I implemented an simplified version of RIP. I maintained a routing table on every host. I chose Trie to easily handle IPmasks and longest prefix match. Every host in network regularly (every 5 seconds) sends its info to adjacent hosts.

``` c
typedef struct TrieNode {
  
  struct in_addr ip;
  struct in_addr mask;

  int hops;
  int attenuate_timer;

  int deviceID;

  struct TrieNode *ch[2];

} TrieNode;
```

`hops` marks the distance between this host and the target subnet. The info with minimum `hops` takes precedence.

To deal with broken hosts, I used the `attenuate_timer` . If the current info is out-dated, it will be replaced by another info (likely with a larger `hops`) . Each time a worse info arrives, `attenuate_timer++` .

Here's the whole replacing policy: (`tmp` is the old info)

```c
if (hops + 1 < tmp->hops || tmp->attenuate_timer >= 5) {
    tmp->ip = ip;
    tmp->mask = mask;
    tmp->hops = hops + 1;
    tmp->attenuate_timer = 0;
    tmp->deviceID = deviceID;
} else if (hops + 1 == tmp->hops) {
    tmp->deviceID = deviceID;
    tmp->attenuate_timer = 0;
} else if (hops + 1 > tmp->hops && tmp->hops > 1) {
    // tmp->hops = 0 means this is local IP, or this is manually added(with highest priority)
    // tmp->hops = 1 means this is direct link, don't change
    tmp->attenuate_timer++;
}
```

I used protocol ID `0XFE` to mark these RIP packets. And for simplicity, I compressed each info to 12 bytes.

![image-20231025201534384](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025201534384.png)

### Checkpoint 3

In this network, `ns1` regularly sends IPv4 packets to `ns6` , with protocol ID `0XFD`: (the wireshark monitors vnet6-5 on ns6)

```
1 - 2 - 3 - 4
    |   |
    5 - 6

(below is fed to makeVNet)
6
1 2 10.100.1
2 3 10.100.2
3 4 10.100.3
2 5 10.100.4
3 6 10.100.5
5 6 10.100.6
```

![image-20231025203819895](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203819895.png)



The first 12 bytes mark the source MAC address and the dest MAC address:

![image-20231025202347600](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202347600.png)

![image-20231025202358482](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202358482.png)

The 13th and 14th bytes are `0X0800`, which shows this is an IPv4 packet.

![image-20231025202423978](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202423978.png)

The 15th byte is `0x45`, `0100 0101` under binary. `0100` shows the IP version is 4, `0101`shows the header length is $5\times 4=20$ bytes.

![image-20231025202547846](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202547846.png)

The 16th byte is `0x00`. It's the TOS byte.

![image-20231025202819433](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202819433.png)

The 17th and 18th bytes are `0x0020` , showing the total packet length is 32 bytes.

![image-20231025202856215](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202856215.png)

The 19th, 20th, 21st, 22nd bytes are all `0x00`, they are identification code and flags, manually set to 0 without fragmentation.

![image-20231025202955512](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025202955512.png)

![image-20231025203002718](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203002718.png)

The 23rd byte is `0x3e`, showing this packet's TTL  is 62.

![image-20231025203204678](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203204678.png)

The 24th byte is `0xfd`, the protocol ID.

![image-20231025203310436](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203310436.png)

The 25th and 26th bytes are `0x0000`, disabling the header checksum.

![image-20231025203411943](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203411943.png)

The following 8 bytes mark the source IP address `10.100.1.1` and the dest IP address `10.100.6.2`.

![image-20231025203502773](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203502773.png)

![image-20231025203546335](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203546335.png)

The following 12 bytes are the payload `Hello World!`

![image-20231025203638764](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203638764.png)

The rest bytes are ethernet padding.

![image-20231025203715248](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025203715248.png)

### Checkpoint 4

`ns1` regularly sends packets to `ns4` .

In the beginning, `ns4` can receive the packets.

![image-20231025213500562](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025213500562.png)

After disconnecting `ns2`, `ns4` stops receiving packets.

After reconnecting `ns2`, `ns4` can receive the packets.

![image-20231025213558043](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025213558043.png)

### Checkpoint 5

The distances are:

$1-2:1, \ 2-3:1, \dots$ 

Disconnecting `ns5` doesn't change distance between any pair of hosts.

For example, to measure the distance between `ns1` and `ns6` , `ns1` regularly sends packets to `ns6`.

In the beginning, the packets travel `ns1 -> ns2 -> ns5 -> ns6`.

![image-20231025204924466](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025204924466.png)

![image-20231025204932109](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025204932109.png)

After disconnecting `ns5` , the packets travel `ns1 -> ns2 -> ns3 -> ns6`

![image-20231025205057195](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025205057195.png)

![image-20231025205107302](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025205107302.png)

Thus the distance between `ns1` and `ns6` is constantly $64+1-\text{TTL}=3$ .

I ran another test which involves change of distance:

```
1 - 2 - 3
|   |   |
4 - 5 - 6

(the following is fed to makeVNet)

6
1 2 10.100.1
2 3 10.100.2
1 4 10.100.3
2 5 10.100.4
3 6 10.100.5
4 5 10.100.6
5 6 10.100.7

```

To measure $dis(1,3)$ , `ns1` regularly sends packets to `ns3`.

In the beginning, the packets travel `ns1 -> ns2 -> ns3`, so the distance is 2.

![image-20231025210243453](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025210243453.png)

After disconnecting `ns2`, the packets travel `ns1 -> ns4 -> ns5 -> ns6 -> ns3`, so the distance is 4.

![image-20231025210430496](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231025210430496.png)

(In my implementation, the recovery needs some iterations, so there will be wrongly forwarded packets for a short period of time after disconnection)

### Checkpoint 6

``` c
int route(const struct in_addr ip) {
  uint32_t rev_ip = ((ip.s_addr & 255) << 24) + 
                    (((ip.s_addr >> 8) & 255) << 16) + 
                    (((ip.s_addr >> 16) & 255) << 8) + 
                    ((ip.s_addr >> 24) & 255);
  TrieNode *tmp = DVTrieRoot;
  for (int i = 31; i >= 0; i--) {
    if (tmp->ch[rev_ip >> i & 1] == NULL)
      break;
    tmp = tmp->ch[rev_ip >> i & 1];
  }
  return tmp->deviceID;
}
```

During a routing procedure, only the IP mask with the longest prefix will match the current IP. While traversing on the Trie, whenever a node has a child, there's a longer prefix match.
