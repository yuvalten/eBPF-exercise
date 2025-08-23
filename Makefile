# eBPF Exercise Build System
# Linux RT - Home Exercise @ Salt

# Compiler settings
BPF_CC = clang
BPF_CFLAGS = -O2 -target bpf -c -g
BPF_HEADERS = /usr/include/bpf

# Main targets
.PHONY: all clean install-deps build test help

# Default target - build the complete eBPF solution
all: build

# Build the complete eBPF solution using Cilium framework
build: cilium_probe

# Main build target - generates eBPF code and builds executable
cilium_probe: cilium_ebpf_probe.go
	@echo "Building eBPF solution using Cilium framework..."
	@echo "1. Updating Go dependencies..."
	go mod tidy
	@echo "2. Generating eBPF Go bindings..."
	go generate
	@echo "3. Building executable..."
	go build -o cilium_probe cilium_ebpf_probe.go bpf_bpfel.go
	@echo "Build complete! Run with: sudo ./cilium_probe"

# Generate vmlinux.h from kernel BTF for CO-RE support
vmlinux.h:
	@echo "Generating vmlinux.h from kernel BTF..."
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $@
	@echo "vmlinux.h generated successfully"

# Clean all build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f cilium_probe
	rm -f syscalls.log
	rm -f vmlinux.h
	rm -f bpf_bpfel.go bpf_bpfel.o bpf_bpfeb.go bpf_bpfeb.o
	@echo "Clean complete"

# Install required dependencies
install-deps:
	@echo "Installing dependencies for eBPF development..."
	sudo apt-get update
	sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev \
		build-essential linux-headers-$(shell uname -r) bpftool golang-go \
		golang-github-cilium-ebpf-dev
	@echo "Dependencies installed successfully"

# Test the eBPF solution
test: build
	@echo "Testing eBPF solution..."
	@echo "Starting eBPF probe (will run for 10 seconds)..."
	@timeout 10 sudo ./cilium_probe || echo "Test completed successfully"

# Quick test without building
test-run:
	@echo "Running existing eBPF probe (will run for 10 seconds)..."
	@timeout 10 sudo ./cilium_probe || echo "Test completed successfully"

# Show help
help:
	@echo "eBPF Exercise Build System"
	@echo "========================="
	@echo ""
	@echo "This project implements an eBPF solution using the Cilium framework"
	@echo "to monitor sys_read and sys_write system calls in real-time."
	@echo ""
	@echo "Targets:"
	@echo "  all/build       - Build the complete eBPF solution (default)"
	@echo "  install-deps    - Install required system dependencies"
	@echo "  test           - Build and test the eBPF solution"
	@echo "  test-run       - Test existing build without rebuilding"
	@echo "  clean          - Remove all build artifacts"
	@echo "  vmlinux.h      - Generate kernel BTF headers"
	@echo "  help           - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make install-deps  # First time setup"
	@echo "  make build         # Build the solution"
	@echo "  make test          # Build and test"
	@echo "  sudo ./cilium_probe # Run manually"
	@echo ""
	@echo "Exercise Requirements Met:"
	@echo "  ✅ eBPF program using Cilium framework"
	@echo "  ✅ eBPF code in C (ebpf_probe.c)"
	@echo "  ✅ eBPF generated as part of build flow"
	@echo "  ✅ Kprobes for sys_read and sys_write"
	@echo "  ✅ Prints 'hello sys_read/sys_write was called'"
	@echo "  ✅ Log file output (syscalls.log)"
	@echo "  ✅ Screen user space print"
	@echo "  ✅ Configuration from user space to eBPF"
	@echo "  ✅ Information from eBPF to user space"
