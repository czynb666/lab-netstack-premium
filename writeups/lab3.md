### Writing Task 5

$1.$

$2$ sessions.

$12, 1652$ segments.

$2.$

$(10.0.0.74,43120,115.27.207.221,80)$

$(10.0.0.74,43122,115.27.207.221,80)$

$3.$

$43520$ . The "window" bytes are $0\text{x}55=85$ , $85\times512=43520$ . 

### Programming Task 4

I divided the whole TCP stack into $4$ header files. `socket.h` is for sockets, `netstack.h` is to set up the network stack, `connection.h` is for maintaining TCP states, `tcp.h` is for reliable byte transmission.

### Run Tasks

Run `sudo make` .

`cd vnetUtils/examples`, `sudo bash ./makeVNet < test.txt`

```
ns1 --- ns2 --- ns3 --- ns4
```

Then open a terminal with every ns hosts.

````bash
cd vnetUtils/helper; 
sudo ./execNS ns* bash
cd ../../build
````

Then `sudo ./router ` on ns2 and ns3.

In checkpoint 9, `sudo ./echo_server` on ns4, `sudo ./echo_client 10.100.3.2` on ns1.

In checkpoint 10, `sudo ./perf_server` on ns4, `sudo ./perf_client 10.100.3.2` on ns1.

### Checkpoint 7

![image-20231115231655139](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115231655139.png)

These bytes stand for the source port, destination port, sequence number, acknowledgment number, header's length, TCP flags, window size, TCP checksum, urgent pointer.

### Checkpoint 8

I used a very brutal emulation for bad links.

Whenever sending an IP packet, there's a chance of simply dropping it.

![image-20231115232003715](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115232003715.png)

This is checkpoint 9 running. The wireshark is monitoring vnet1-2 on ns1. We can see there are retransmissions and dup ACKs.

![image-20231115232241060](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115232241060.png)

### Checkpoint 9

![image-20231115215623194](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115215623194.png)



![image-20231115215634200](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115215634200.png)

### Checkpoint 10

![image-20231115221012698](C:\Users\23512\AppData\Roaming\Typora\typora-user-images\image-20231115221012698.png)