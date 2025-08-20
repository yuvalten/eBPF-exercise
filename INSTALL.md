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
sudo apt-get install -y clang llvm libbpf-dev libelf-dev zlib1g-dev build-essential

# Or use the Makefile target
make install-deps
```

### 2. Build the Project

```bash
# Build both eBPF program and user space application
make

# Or build individually
make ebpf_probe    # Builds the eBPF program
make user_program  # Builds the user space application
```

### 3. Verify Installation

```bash
# Check if files were created
ls -la ebpf_probe user_program

# Make test script executable
chmod +x test_probe.sh
```

## Usage

### Basic Usage

```bash
# Run with default settings
sudo ./user_program

# Run with verbose mode
sudo ./user_program -v

# Run with custom message
sudo ./user_program -m "Custom probe message"

# Run with both verbose mode and custom message
sudo ./user_program -v -m "Verbose custom message"
```

### Command Line Options

- `-v, --verbose`: Enable verbose mode (prints to trace pipe)
- `-m, --message`: Set custom message (max 63 characters)
- `-h, --help`: Show help message

### Testing the Probe

```bash
# Run the automated test script
sudo ./test_probe.sh

# Or test manually by running the probe and generating activity
sudo ./user_program -v &
# In another terminal, run commands like:
cat /proc/version
echo "test" > /tmp/test.txt
cat /tmp/test.txt
```

## How It Works

### eBPF Program (`ebpf_probe.c`)

1. **Kprobes**: Attaches to `sys_read` and `sys_write` kernel functions
2. **Configuration**: Receives settings from user space via BPF map
3. **Event Collection**: Captures process information (PID, command name, timestamp)
4. **Data Transfer**: Sends events to user space via perf events
5. **Logging**: Optionally prints to trace pipe when verbose mode is enabled

### User Space Program (`user_program.c`)

1. **Program Loading**: Loads and verifies the eBPF program
2. **Kprobe Attachment**: Attaches kprobes to kernel functions
3. **Configuration**: Passes settings to eBPF program
4. **Event Handling**: Receives and displays events from eBPF
5. **Signal Handling**: Gracefully shuts down on Ctrl+C

### Data Flow

```
User Space → Configuration Map → eBPF Program
     ↑                              ↓
Event Display ← Perf Events ← Kernel Function Calls
```

## Troubleshooting

### Common Issues

1. **Permission Denied**
   ```bash
   # Must run as root
   sudo ./user_program
   ```

2. **Failed to Load eBPF Object**
   ```bash
   # Check if dependencies are installed
   make install-deps
   # Rebuild
   make clean && make
   ```

3. **No Events Displayed**
   ```bash
   # Check if kprobes are attached
   cat /sys/kernel/debug/tracing/available_kprobes | grep sys_read
   cat /sys/kernel/debug/tracing/available_kprobes | grep sys_write
   ```

4. **Build Errors**
   ```bash
   # Ensure clang and libbpf are installed
   sudo apt-get install -y clang llvm libbpf-dev
   ```

### Debug Mode

```bash
# Enable debug output
sudo ./user_program -v

# Check trace pipe in another terminal
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

## Cleanup

```bash
# Remove build artifacts
make clean

# Remove temporary files
rm -f /tmp/test_probe.txt
```

## Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   User Space    │    │   eBPF VM      │    │   Kernel       │
│   Application   │◄──►│   Program      │◄──►│   Functions    │
│                 │    │                 │    │                 │
│ • Load eBPF    │    │ • Kprobes      │    │ • sys_read     │
│ • Configure    │    │ • Event capture│    │ • sys_write    │
│ • Handle events│    │ • Data transfer│    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Performance Considerations

- The eBPF program runs in kernel space with minimal overhead
- Perf events provide efficient data transfer to user space
- Configuration changes are applied immediately without reloading
- The program can handle high-frequency system calls efficiently

## Security Notes

- Requires root privileges to load eBPF programs
- eBPF programs are verified by the kernel before execution
- Only GPL-licensed programs can access kernel functions
- Resource limits are enforced by the kernel


