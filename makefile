SRC_DIR = src
BUILD_DIR = build

CC = gcc
CFLAGS = -lpcap -I$(SRC_DIR)

ETHERNET_DIR = $(SRC_DIR)/ethernet
IP_DIR = $(SRC_DIR)/ip
TESTS_DIR = $(SRC_DIR)/tests


OBJS = $(wildcard $(BUILD_DIR)/*.o) 

cp12: init
	$(CC) $(TESTS_DIR)/sender.c $(OBJS) -o $(BUILD_DIR)/sender $(CFLAGS)
	$(CC) $(TESTS_DIR)/receiver.c $(OBJS) -o $(BUILD_DIR)/receiver $(CFLAGS)
	$(CC) $(TESTS_DIR)/ns1.c $(OBJS) -o $(BUILD_DIR)/ns1 $(CFLAGS)
	$(CC) $(TESTS_DIR)/ns2.c $(OBJS) -o $(BUILD_DIR)/ns2 $(CFLAGS)
	$(CC) $(TESTS_DIR)/ns3.c $(OBJS) -o $(BUILD_DIR)/ns3 $(CFLAGS)

init:
	-mkdir build
	make -C $(ETHERNET_DIR)
	make -C $(IP_DIR)

.PHONY: clean
clean:
	rm -r build