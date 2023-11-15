OUTPUT_DIR = ../../build
TARGET = $(OUTPUT_DIR)/tcp.o
CC = gcc
CFLAGS = -lpcap -I..

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(notdir $(SRC)))

all: $(OBJ)

$(OUTPUT_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)