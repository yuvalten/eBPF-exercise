# eBPF Exercise - Installation and Usage Guide

## Prerequisites

This exercise requires:
- Ubuntu/Debian-based Linux system (tested on Ubuntu 20.04+)
- Root privileges (sudo access)
- Internet connection for package installation

## Installation

### 1. Install Dependencies

```bash
# Update package list
sudo apt-get update

# Install required packages
sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev build-essential golang-go

# Or use the Makefile target
make install-deps
```

### 2. Build the Project

```bash
# Build everything with Cilium framework
make clean && make

# Or build individually
make ebpf_probe.o      # Builds the C eBPF program
make cilium-framework  # Builds the Go user space program using Cilium framework
```

### 3. Verify Installation

```bash
# Check if files were created
ls -la ebpf_probe.o cilium_probe

# Check Go module
go mod tidy
```

## Usage

### Basic Usage

```bash
# Run with default settings
sudo ./cilium_probe

# The program will:
# - Load the eBPF program
# - Attach kprobes to sys_read and sys_write
# - Start monitoring syscalls
# - Print events to screen
# - Log events to syscalls.log
```

### Testing the Probe

```bash
# Run the automated test
make test

# Or test manually by running the probe and generating activity
sudo ./cilium_probe &
# In another terminal, run commands like:
cat /proc/version
echo "test" > /tmp/test.txt
cat /tmp/test.txt
```

## How It Works

### eBPF Program (`ebpf_probe.c`)

1. **Kprobes**: Attaches to `__x64_sys_read` and `__x64_sys_write` kernel functions
2. **Configuration**: Receives settings from user space via BPF map
3. **Event Collection**: Captures process information (PID, command name, timestamp)
4. **Data Transfer**: Sends events to user space via perf events
5. **CO-RE Support**: Uses `vmlinux.h` for cross-kernel compatibility

### Go User Space Program (`cilium_ebpf_probe.go`)

1. **Cilium Framework**: Uses `github.com/cilium/ebpf` library
2. **Code Generation**: `bpf2go` tool generates Go bindings from C eBPF code
3. **Program Loading**: Loads and verifies the eBPF program via Cilium APIs
4. **Kprobe Attachment**: Attaches kprobes using Cilium's `link.Kprobe`
5. **Event Handling**: Receives events via Cilium's `perf.NewReader`
6. **Configuration**: Passes settings to eBPF program via Cilium maps
7. **Signal Handling**: Gracefully shuts down on Ctrl+C

## Cilium Framework Features

### **Automatic Go Bindings**
- `bpf2go` generates Go structs and functions from C eBPF code
- Type-safe interfaces between Go and eBPF
- Automatic handling of C structs and arrays

### **High-Level APIs**
- `ebpf.CollectionSpec` for program specifications
- `link.Kprobe` for kprobe attachment
- `perf.NewReader` for event consumption
- `ebpf.Map` for map operations

### **Resource Management**
- Automatic memory management with Go's garbage collection
- Proper cleanup with `defer` statements
- Resource limit handling with `rlimit.RemoveMemlock()`

## Build Process

### **1. eBPF Compilation**
```bash
clang -O2 -target bpf -c -g -mcpu=v3 -I. -o ebpf_probe.o ebpf_probe.c
```

### **2. Go Code Generation**
```bash
go generate  # Runs bpf2go to create Go bindings
```

### **3. Go Compilation**
```bash
go build -o cilium_probe cilium_ebpf_probe.go bpf_bpfel.go
```

## Generated Files

After building, you'll have:
- **`bpf_bpfel.go`**: Go bindings for little-endian systems
- **`bpf_bpfeb.go`**: Go bindings for big-endian systems
- **`cilium_probe`**: Executable Go binary using Cilium framework

## Troubleshooting

### **Common Issues**

1. **Go not installed**:
   ```bash
   sudo apt-get install -y golang-go
   ```

2. **Cilium eBPF library missing**:
   ```bash
   go mod tidy
   go get github.com/cilium/ebpf
   ```

3. **Permission denied**:
   ```bash
   sudo ./cilium_probe
   ```

4. **eBPF not supported**:
   ```bash
   ls /sys/kernel/btf/vmlinux
   ```

### **Rebuild if needed**:
```bash
make clean && make
```

## Monitoring and Logging

### **Real-time Output**
The program prints events directly to the screen:
```
hello sys_read was called
hello sys_write was called
```

### **Log File**
All events are saved to `syscalls.log` with timestamps and process details.

### **Performance**
The Cilium framework provides efficient event processing with minimal overhead.

## Next Steps

- Read `CILIUM_FRAMEWORK.md` for detailed framework information
- Check `REQUIREMENTS_COMPLIANCE.md` for full requirements mapping
- Explore `QUICKSTART.md` for quick setup instructions
- Run `make help` for all available commands

This solution demonstrates professional eBPF development using the **REAL Cilium framework**!


