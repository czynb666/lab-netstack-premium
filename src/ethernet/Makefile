OUTPUT_DIR = ../../build
TARGET = $(OUTPUT_DIR)/ethernet.o
CC := gcc

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,$(OUTPUT_DIR)/%.o,$(notdir $(SRC)))

$(TARGET):$(SRC)
	$(CC) -c $< -o $@