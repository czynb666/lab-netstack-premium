OUTPUT_DIR = ../../build
INCLUDE_DIR = ../
TARGET = $(OUTPUT_DIR)/tests.o
CC = gcc
CFLAGS = -lpcap -I$(INCLUDE_DIR)

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(notdir $(SRC)))

$(TARGET):$(SRC)
	$(CC) -c $< -o $@ $(CFLAGS)