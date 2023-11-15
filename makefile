SRC_DIR = src
BUILD_DIR = build

CC = gcc
CFLAGS = -O2 -lpcap -I$(SRC_DIR) -Wl,--wrap,socket -Wl,--wrap,listen -Wl,--wrap,bind -Wl,--wrap,accept -Wl,--wrap,connect -Wl,--wrap,read -Wl,--wrap,write -Wl,--wrap,close -Wl,--wrap,getaddrinfo

ETHERNET_DIR = $(SRC_DIR)/ethernet
IP_DIR = $(SRC_DIR)/ip
TCP_DIR = $(SRC_DIR)/tcp
TESTS_DIR = $(SRC_DIR)/tests
CP_DIR = checkpoints

OBJS = $(wildcard $(BUILD_DIR)/*.o) 

cp12: init
	$(CC) $(TESTS_DIR)/router.c $(OBJS) -o $(BUILD_DIR)/router $(CFLAGS)

	$(CC) $(CP_DIR)/echo_client.c $(CP_DIR)/unp.c $(OBJS) -o $(BUILD_DIR)/echo_client $(CFLAGS)
	$(CC) $(CP_DIR)/echo_server.c $(CP_DIR)/unp.c $(OBJS) -o $(BUILD_DIR)/echo_server $(CFLAGS)
	$(CC) $(CP_DIR)/perf_client.c $(CP_DIR)/unp.c $(OBJS) -o $(BUILD_DIR)/perf_client $(CFLAGS)
	$(CC) $(CP_DIR)/perf_server.c $(CP_DIR)/unp.c $(OBJS) -o $(BUILD_DIR)/perf_server $(CFLAGS)

init:
	-mkdir build
	make -C $(ETHERNET_DIR)
	make -C $(IP_DIR)
	make -C $(TCP_DIR)

.PHONY: clean
clean:
	rm -r build