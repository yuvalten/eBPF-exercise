CC = gcc
CFLAGS = -Wall -Wextra -O2
BPF_CC = clang
BPF_CFLAGS = -O2 -target bpf -c -g
LIBBPF_DIR = /usr/include
BPF_HEADERS = /usr/include/bpf

# Default target
all: ebpf_probe.o user_program

# Build eBPF program with BTF support
ebpf_probe.o: ebpf_probe.c
	$(BPF_CC) $(BPF_CFLAGS) -I$(BPF_HEADERS) $< -o $@

# Build user space program
user_program: user_program.c
	$(CC) $(CFLAGS) -I$(LIBBPF_DIR) $< -o $@ -lbpf -lelf -lz

# Clean build artifacts
clean:
	rm -f ebpf_probe ebpf_probe.o user_program *.o

# Install dependencies (for Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev build-essential linux-headers-$(shell uname -r)

.PHONY: all clean install-deps
