# Linux RT - eBPF Exercise Implementation

## Overview

This project implements a complete eBPF (Extended Berkeley Packet Filter) solution using the Cilium framework to monitor system calls (`sys_read` and `sys_write`) in real-time. The implementation demonstrates modern eBPF development practices with proper user space integration.

## Features

- **Real-time system call monitoring** using kprobes
- **Cilium framework integration** for modern eBPF development
- **Bidirectional communication** between user space and kernel space
- **Configurable logging** with both console and file output
- **Graceful signal handling** for clean termination
- **Comprehensive build system** with automatic eBPF generation

## Architecture

### Components

1. **eBPF Program** (`ebpf_probe.c`)
   - Written in C using eBPF helpers
   - Attaches kprobes to `__x64_sys_read` and `__x64_sys_write`
   - Collects process information (PID, TGID, command name)
   - Sends events to user space via perf ring buffer

2. **User Space Program** (`cilium_ebpf_probe.go`)
   - Written in Go using Cilium eBPF framework
   - Loads and attaches eBPF programs
   - Processes events from kernel space
   - Provides configuration interface
   - Handles logging and signal termination

3. **Build System** (`Makefile`)
   - Automated eBPF compilation and generation
   - Go module management
   - Clean build artifacts



## Installation & Setup Guide

This guide provides detailed instructions for setting up the eBPF development environment from scratch on an EC2 instance (tested on Ubuntu 24.04.2 LTS).

### Step 1: Connect to Your EC2 Instance

```bash
# Connect via SSH (replace with your key and IP)
ssh -i your-key.pem ubuntu@<ip>
e.g ssh -i "C:\Users\Tyuva\Downloads\yuval_salt.pem" ubuntu@13.51.150.11
```

### Step 2: Update System Packages

```bash
# Update package lists and upgrade existing packages
sudo apt-get update
sudo apt-get upgrade -y
```

### Step 3: Install Required Dependencies

```bash
# Install essential build tools and development packages
sudo apt-get install -y \
    build-essential \
    clang \
    llvm \
    libbpf-dev \
    libelf-dev \
    zlib1g-dev \
    linux-headers-$(uname -r) \
    linux-tools-$(uname -r) \
    golang-go \
    git \
    make \
    curl \
    wget

git clone --recurse-submodules https://github.com/libbpf/bpftool.git
make -C ~/bpftool/src -j"$(nproc)"
sudo ~/bpftool/src/bpftool version

sudo install -m 0755 ~/bpftool/src/bpftool /usr/local/bin/bpftool
command -v bpftool
echo 'export PATH="/usr/local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc


# Verify installations
clang --version
go version
bpftool --version
```

**Expected output:**
```
Ubuntu clang version 18.1.3 (1ubuntu1)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/bin
go version go1.22.2 linux/amd64
bpftool v7.6.0
using libbpf v1.6
features: llvm, skeletons
```

### Step 4: Install Cilium eBPF Framework

```bash
# Install Cilium eBPF Go library
go install github.com/cilium/ebpf/cmd/bpf2go@latest
echo 'export PATH="$HOME/go/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Verify installation
bpf2go --help
```

### Step 5: Clone and Setup Project

```bash
# Create project directory
mkdir ~/github-repo
cd ~/github-repo

# Clone the repository
git clone https://github.com/yuvalten/eBPF-exercise/tree/ebpf_exercise

# Verify project structure
ll
```

### Step 6: Initialize Go Module

```bash
# Initialize Go module (if not already done)
#go mod init cilium-ebpf-probe

# Add Cilium eBPF dependency
go get github.com/cilium/ebpf@latest

# Verify go.mod file
cat go.mod
```

### Step 7: Generate Kernel Headers

```bash
# Generate vmlinux.h for CO-RE (Compile Once, Run Everywhere) support
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

# Verify file was created
ls -la vmlinux.h
```

**Expected output:**
```
-rw-r--r-- 1 ubuntu ubuntu 3145728 Aug 23 15:10 vmlinux.h
```

### Step 8: Build the Project

```bash
# Build the complete eBPF solution
make build
```

**Expected output:**
```
Building eBPF solution using Cilium framework...
1. Updating Go dependencies...
go mod tidy
2. Generating eBPF Go bindings...
go generate
Compiled /home/ubuntu/github-repo/bpf_bpfel.o
Stripped /home/ubuntu/github-repo/bpf_bpfel.o
Wrote /home/ubuntu/github-repo/bpf_bpfel.go
Compiled /home/ubuntu/github-repo/bpf_bpfeb.o
Stripped /home/ubuntu/github-repo/bpf_bpfeb.o
Wrote /home/ubuntu/github-repo/bpf_bpfeb.go
3. Building executable...
go build -o cilium_probe cilium_ebpf_probe.go bpf_bpfel.go
Build complete! Run with: sudo ./cilium_probe
```

### Step 9: Verify Build Success

```bash
# Check generated files
ls -la *.go *.o cilium_probe

# Verify executable
file cilium_probe
```

**Expected output:**
```
-rw-rw-r-- 1 ubuntu ubuntu    3403 Aug 23 20:18 bpf_bpfeb.go
-rw-rw-r-- 1 ubuntu ubuntu    7352 Aug 23 20:18 bpf_bpfeb.o
-rw-rw-r-- 1 ubuntu ubuntu    3462 Aug 23 20:18 bpf_bpfel.go
-rw-rw-r-- 1 ubuntu ubuntu    7352 Aug 23 20:18 bpf_bpfel.o
-rw-rw-r-- 1 ubuntu ubuntu    6797 Aug 23 17:59 cilium_ebpf_probe.go
-rwxrwxr-x 1 ubuntu ubuntu 5221211 Aug 23 20:18 cilium_probe
-rw-rw-r-- 1 ubuntu ubuntu   14200 Aug 23 13:08 ebpf_probe.o

cilium_probe: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, Go BuildID=..., not stripped
```

### Step 10: Test the eBPF Probe

```bash
# Run a quick test (10 seconds)
make test
```

**Expected output:**
```
Testing eBPF solution...
Starting eBPF probe (will run for 10 seconds)...
Cilium Framework eBPF Probe
============================
Verbose mode: enabled
Press Ctrl+C to stop

eBPF program loaded and attached successfully!
Monitoring sys_read and sys_write calls...

hello sys_read was called
hello sys_write was called
hello sys_read was called
...
Test completed successfully
```

### Step 11: Run the Full Program

```bash
# Run the eBPF probe (will run until Ctrl+C)
sudo ./cilium_probe
```

**To stop the program:** Press `Ctrl+C`

**Expected behavior:**
- Program loads eBPF programs into kernel
- Monitors all `sys_read` and `sys_write` calls
- Prints "hello sys_read was called" or "hello sys_write was called"
- Logs detailed information to `syscalls.log`
- Gracefully shuts down on Ctrl+C

## Configuration

The program supports configuration through the `Config` struct:

```go
type Config struct {
    Verbose uint32 // Verbose logging level (0=disabled, 1=enabled)
}
```

Configuration is passed from user space to the eBPF program via a map.

## Implementation Details

### eBPF Program (`ebpf_probe.c`)

- Uses kprobes to hook into system call entry points
- Collects process context information
- Sends structured events to user space
- Supports configuration via maps

### User Space Program (`cilium_ebpf_probe.go`)

- Loads eBPF bytecode using Cilium framework
- Manages perf ring buffer for event reception
- Processes events in real-time
- Provides graceful shutdown handling

### Data Flow

1. **System call occurs** → Kernel executes `sys_read` or `sys_write`
2. **eBPF program triggers** → Kprobe fires, eBPF code executes
3. **Event collection** → Process info, timestamps, function names captured
4. **User space notification** → Event sent via perf ring buffer
5. **Event processing** → User space program receives and logs event

## Build Process

The build process automatically:

1. **Updates Go dependencies** (`go mod tidy`)
2. **Generates eBPF Go bindings** (`go generate`)
   - Compiles `ebpf_probe.c` → `bpf_bpfel.o` (eBPF bytecode)
   - Creates `bpf_bpfel.go` (Go bindings for eBPF objects)
3. **Builds the final executable** (`go build`)

## Logging

- **Console output**: Real-time system call notifications
- **File logging**: Detailed event information in `syscalls.log`
- **Error handling**: Comprehensive error reporting and recovery

## Performance Considerations

- **Perf ring buffer**: Efficient kernel-user space communication
- **Event batching**: Optimized for high-frequency events
- **Memory management**: Proper cleanup and resource management

### Debug Mode

Enable verbose logging by setting `Verbose: 1` in the configuration.
