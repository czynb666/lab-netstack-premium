SRC_DIR = src
BUILD_DIR = build

CC = gcc
CFLAGS = -lpcap -I$(SRC_DIR)

ETHERNET_DIR = $(SRC_DIR)/ethernet
TESTS_DIR = $(SRC_DIR)/tests

OBJ = $(wildcard $(BUILD_DIR)/*.o)

TARGET_CP1 = cp1


all:
	
$(TARGET_CP1):$(OBJ)
	make init
	$(CC) $^ -o $@ $(CFLAGS)


init:
	-mkdir build
	make -C $(ETHERNET_DIR)
	make -C $(TESTS_DIR)
