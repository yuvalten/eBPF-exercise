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

## Requirements

- Linux kernel 4.18+ (for eBPF support)
- Go 1.19+
- Clang/LLVM for eBPF compilation
- Root privileges (for eBPF operations)

## Installation

### 1. Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev \
    build-essential linux-headers-$(uname -r) bpftool golang-go \
    golang-github-cilium-ebpf-dev
```

### 2. Clone and Build

```bash
git clone <repository-url>
cd <repository-name>
make cilium-framework
```

## Usage

### Run the eBPF Probe

```bash
# Run with sudo (required for eBPF operations)
sudo ./cilium_probe
```

### Expected Output

The program will:
1. Load and attach eBPF programs to kernel functions
2. Print "hello sys_read was called" or "hello sys_write was called" for each system call
3. Log detailed information to `syscalls.log`
4. Handle Ctrl+C gracefully for clean termination

### Configuration

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

1. Compiles eBPF C code to bytecode
2. Generates Go bindings for eBPF objects
3. Builds the final executable

```bash
make cilium-framework
```

## Signal Handling

The program properly handles termination signals:

- **SIGINT (Ctrl+C)**: Graceful shutdown
- **SIGTERM**: Clean termination
- **Resource cleanup**: Properly detaches eBPF programs and closes file descriptors

## Logging

- **Console output**: Real-time system call notifications
- **File logging**: Detailed event information in `syscalls.log`
- **Error handling**: Comprehensive error reporting and recovery

## Performance Considerations

- **Perf ring buffer**: Efficient kernel-user space communication
- **Event batching**: Optimized for high-frequency events
- **Memory management**: Proper cleanup and resource management

## Troubleshooting

### Common Issues

1. **Permission denied**: Ensure running with sudo
2. **eBPF not supported**: Check kernel version (4.18+ required)
3. **Build failures**: Verify all dependencies are installed

### Debug Mode

Enable verbose logging by setting `Verbose: 1` in the configuration.

## Development

### Project Structure

```
.
.
├── ebpf_probe.c          # eBPF C program
├── vmlinux.h             # BTF headers for CO-RE (required for build)
├── cilium_ebpf_probe.go  # User space Go program
├── bpf_bpfel.go          # Auto-generated Go bindings (Little Endian) ← bpf2go
├── bpf_bpfeb.go          # Auto-generated Go bindings (Big Endian)   ← bpf2go
├── bpf_bpfel.o           # eBPF bytecode (ELF, LE)                  ← build artifact
├── bpf_bpfeb.o           # eBPF bytecode (ELF, BE)                  ← build artifact
├── go.mod                # Go module definition (dependencies)
├── go.sum                # Checksums for dependencies
├── Makefile              # Build flow: go generate → go build
├── README.md             # Documentation
└── .gitignore            # Recommended: ignore binary, logs, and *.o

```

### Adding New Features

1. **New system calls**: Add kprobes in Go code and corresponding eBPF functions
2. **Additional data**: Extend the `Event` struct and eBPF data collection
3. **Configuration**: Add new fields to the `Config` struct

## License

GPL
