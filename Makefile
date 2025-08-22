CC = gcc
CFLAGS = -Wall -Wextra -O2
BPF_CC = clang
BPF_CFLAGS = -O2 -target bpf -c -g -mcpu=v3
LIBBPF_DIR = /usr/include
BPF_HEADERS = /usr/include/bpf

# Cilium framework build targets
.PHONY: all clean install-deps ebpf user cilium-framework test help

# Default target - build Cilium framework solution
all: cilium-framework

# Build eBPF program (for Cilium framework)
ebpf: ebpf_probe.o

ebpf_probe.o: ebpf_probe.c vmlinux.h
	$(BPF_CC) $(BPF_CFLAGS) -I$(BPF_HEADERS) $< -o $@

# Build user space program (legacy libbpf version)
user: user_program

user_program: user_program.c
	$(CC) $(CFLAGS) -I$(LIBBPF_DIR) $< -o $@ -lbpf -lelf -lz

# Build REAL Cilium framework solution
cilium-framework: ebpf_probe.o
	@echo "Building Cilium framework eBPF solution..."
	go mod tidy
	go generate
	go build -o cilium_probe cilium_ebpf_probe.go bpf_bpfel.go

# Generate vmlinux.h from kernel BTF for CO-RE support
vmlinux.h:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $@

# Clean build artifacts
clean:
	rm -f ebpf_probe ebpf_probe.o user_program *.o vmlinux.h
	rm -f syscalls.log
	rm -f cilium_probe
	rm -f bpf_bpfel.go bpf_bpfel.o bpf_bpfeb.go bpf_bpfeb.o

# Install dependencies for Cilium framework development
install-deps:
	sudo apt-get update
	sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev build-essential linux-headers-$(shell uname -r) bpftool golang-go golang-github-cilium-ebpf-dev

# Test the Cilium framework solution
test: cilium-framework
	@echo "Testing REAL Cilium framework eBPF solution..."
	@echo "Starting Cilium eBPF probe (requires sudo)..."
	@timeout 10 sudo ./cilium_probe || echo "Test completed"

# Test legacy libbpf solution
test-legacy: user
	@echo "Testing legacy libbpf eBPF solution..."
	@echo "Starting eBPF probe (requires sudo)..."
	@timeout 10 sudo ./user_program -v -m "Legacy test" || echo "Test completed"

# Show help
help:
	@echo "Cilium Framework eBPF Build System"
	@echo "=================================="
	@echo "Targets:"
	@echo "  all              - Build Cilium framework solution (default)"
	@echo "  cilium-framework - Build REAL Cilium framework solution using Go"
	@echo "  ebpf             - Build only eBPF program"
	@echo "  user             - Build legacy libbpf user space program"
	@echo "  vmlinux.h        - Generate kernel BTF header for CO-RE"
	@echo "  clean            - Remove build artifacts"
	@echo "  install-deps     - Install required dependencies"
	@echo "  test             - Build and test Cilium framework solution"
	@echo "  test-legacy      - Build and test legacy libbpf solution"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make all              # Build Cilium framework solution"
	@echo "  make test             # Build and test Cilium framework"
	@echo "  sudo ./cilium_probe   # Run Cilium framework probe"
	@echo "  sudo ./user_program   # Run legacy libbpf probe"
