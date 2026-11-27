CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=c11 -D_GNU_SOURCE
LDFLAGS :=

# File pustaka inti (Bagian 2 & 3)
LIB_SRCS := iolib.c banner.c
LIB_HDRS := iolib.h banner.h
LIB_OBJS := $(LIB_SRCS:.c=.o)

# Daftar binary program pengujian
TARGETS  := workload durability_bench ext_inode

.PHONY: all clean setup teardown run-env

all: $(TARGETS)

# Rule kompilasi object file pustaka
%.o: %.c $(LIB_HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Link masing-masing program dengan iolib.o dan banner.o
workload: workload.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

durability_bench: durability_bench.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Helper untuk menjalankan skrip environment lab
setup:
	chmod +x setup_lab.sh teardown_lab.sh
	./setup_lab.sh

teardown:
	./teardown_lab.sh

# Bersihkan hasil kompilasi dan file output sementara
clean:
	rm -f $(LIB_OBJS) $(TARGETS) output_append.txt output_overwrite.txt