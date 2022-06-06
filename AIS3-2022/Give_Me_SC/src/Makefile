TARGET := give_me_sc

CROSS_COMPILE ?= aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

C_FILES = $(wildcard *.c)
O_FILES = $(C_FILES:%.c=%.o)

all: $(TARGET)

$(TARGET): $(O_FILES)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(TARGET)