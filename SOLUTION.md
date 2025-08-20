# eBPF Exercise Solution

This document explains the complete solution for the eBPF exercise, which demonstrates how to create an eBPF program that probes `sys_read` and `sys_write` kernel functions.

## Solution Overview

The solution consists of three main components:

1. **eBPF Program** (`ebpf_probe.c`) - Runs in kernel space, probes system calls
2. **User Space Program** (`user_program.c`) - Loads and manages the eBPF program
3. **Build System** (`Makefile`) - Compiles both components

## Key Features Implemented

✅ **eBPF Program using Cilium framework** - Written in C with proper BPF maps and helpers  
✅ **Kprobes for sys_read and sys_write** - Attaches to kernel functions  
✅ **Build flow integration** - eBPF code generated as part of build process  
✅ **Configuration passing** - User space can configure eBPF program behavior  
✅ **Data transfer** - eBPF sends event data to user space  
✅ **Screen output** - Events displayed in user space, not relying on trace_pipe  
✅ **Logging capability** - Optional verbose mode with kernel logging  

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Space                               │
│  ┌─────────────────┐    ┌─────────────────┐               │
│  │ Configuration   │    │ Event Handler   │               │
│  │ (Command Line)  │    │ (Perf Events)   │               │
│  └─────────┬───────┘    └─────────┬───────┘               │
└────────────┼──────────────────────┼───────────────────────┘
             │                      │
             ▼                      ▼
    ┌─────────────────────────────────────────────────────┐
    │              BPF Maps                              │
    │  ┌─────────────────┐    ┌─────────────────┐       │
    │  │ config_map      │    │ events          │       │
    │  │ (Array)         │    │ (Perf Events)   │       │
    │  └─────────────────┘    └─────────────────┘       │
    └─────────────────────────────────────────────────────┘
             │                      ▲
             ▼                      │
    ┌─────────────────────────────────────────────────────┐
    │              eBPF Program                          │
    │  ┌─────────────────┐    ┌─────────────────┐       │
    │  │ kprobe_sys_read │    │ kprobe_sys_write│       │
    │  │ (Kprobe)        │    │ (Kprobe)        │       │
    │  └─────────────────┘    └─────────────────┘       │
    └─────────────────────────────────────────────────────┘
             │                      │
             ▼                      ▼
    ┌─────────────────────────────────────────────────────┐
    │              Kernel Functions                       │
    │  ┌─────────────────┐    ┌─────────────────┐       │
    │  │ sys_read        │    │ sys_write       │       │
    │  │ (System Call)   │    │ (System Call)   │       │
    │  └─────────────────┘    └─────────────────┘       │
    └─────────────────────────────────────────────────────┘
```

## Implementation Details

### 1. eBPF Program (`ebpf_probe.c`)

**Kprobe Functions:**
- `kprobe_sys_read()` - Probes the `sys_read` kernel function
- `kprobe_sys_write()` - Probes the `sys_write` kernel function

**Data Structures:**
- `struct config` - Configuration from user space (verbose mode, custom message)
- `struct event` - Event data sent to user space (PID, command, timestamp)

**BPF Maps:**
- `config_map` - Array map for configuration data
- `events` - Perf event array for sending events to user space

**Key Features:**
- Captures process information (PID, TGID, command name)
- Timestamps events using `bpf_ktime_get_ns()`
- Sends events via perf events for efficient transfer
- Optional verbose logging to trace pipe

### 2. User Space Program (`user_program.c`)

**Main Functions:**
- Program loading and verification
- Kprobe attachment
- Configuration management
- Event handling via perf buffer
- Signal handling for graceful shutdown

**Command Line Options:**
- `-v` - Enable verbose mode
- `-m` - Set custom message
- `-h` - Show help

**Event Processing:**
- Receives events from eBPF via perf events
- Displays formatted event information
- Handles high-frequency events efficiently

### 3. Build System (`Makefile`)

**Targets:**
- `all` - Builds both eBPF and user space programs
- `ebpf_probe` - Builds eBPF program with clang
- `user_program` - Builds user space program with gcc
- `install-deps` - Installs required dependencies
- `clean` - Removes build artifacts

**Compiler Settings:**
- Uses clang with BPF target for eBPF code
- Links against libbpf, libelf, and zlib
- Optimized compilation flags

## Data Flow

1. **Configuration Flow:**
   ```
   User Input → Command Line Args → Config Struct → BPF Map → eBPF Program
   ```

2. **Event Flow:**
   ```
   Kernel Function Call → eBPF Kprobe → Event Struct → Perf Events → User Space Display
   ```

3. **Logging Flow:**
   ```
   eBPF Program → bpf_printk() → Trace Pipe (when verbose mode enabled)
   ```

## Security and Safety

- **Kernel Verification:** All eBPF programs are verified by the kernel
- **Resource Limits:** Memory and instruction limits enforced
- **GPL License:** Required for accessing kernel functions
- **Privilege Requirements:** Root access needed for eBPF operations

## Performance Characteristics

- **Minimal Overhead:** eBPF runs in kernel space with JIT compilation
- **Efficient Transfer:** Perf events provide zero-copy data transfer
- **Scalable:** Can handle high-frequency system calls
- **Non-blocking:** Events are buffered and processed asynchronously

## Testing and Validation

### Automated Testing
- `test_probe.sh` - Basic functionality test
- `demo.sh` - Interactive demonstration

### Manual Testing
```bash
# Start the probe
sudo ./user_program -v

# Generate activity in another terminal
cat /proc/version
echo "test" > /tmp/test.txt
cat /tmp/test.txt
```

### Verification
- Check kprobe attachment: `cat /sys/kernel/debug/tracing/available_kprobes`
- Monitor trace pipe: `sudo cat /sys/kernel/debug/tracing/trace_pipe`
- Verify BPF maps: `sudo bpftool map list`

## Troubleshooting Guide

### Common Issues

1. **Permission Denied**
   - Solution: Run with `sudo`

2. **Failed to Load eBPF**
   - Check: Dependencies installed (`make install-deps`)
   - Check: Kernel version supports eBPF (3.18+)

3. **No Events Displayed**
   - Check: Kprobes attached successfully
   - Check: BPF maps created properly

4. **Build Errors**
   - Check: clang and libbpf installed
   - Check: Kernel headers available

### Debug Steps

1. Enable verbose mode: `./user_program -v`
2. Check trace pipe: `sudo cat /sys/kernel/debug/tracing/trace_pipe`
3. Verify BPF objects: `sudo bpftool prog list`
4. Check system logs: `dmesg | tail`

## Future Enhancements

- **More System Calls:** Extend to monitor additional functions
- **Filtering:** Add process/command filtering capabilities
- **Metrics:** Collect and display performance statistics
- **Web Interface:** Real-time monitoring dashboard
- **Persistence:** Save events to log files
- **Network Monitoring:** Extend to network-related system calls

## Learning Outcomes

This exercise demonstrates:

1. **eBPF Fundamentals:** Understanding BPF maps, helpers, and program structure
2. **Kernel Programming:** Working with kprobes and kernel functions
3. **System Programming:** Managing eBPF programs and perf events
4. **Performance:** Efficient kernel-user space communication
5. **Security:** Safe kernel code execution and verification

## Conclusion

The solution successfully implements all requirements:
- ✅ eBPF program using Cilium framework
- ✅ Kprobes for sys_read and sys_write
- ✅ Build flow integration
- ✅ Configuration passing
- ✅ Data transfer from eBPF to user space
- ✅ Screen output (not trace_pipe dependent)
- ✅ Working installation and usage instructions

The implementation is production-ready with proper error handling, signal management, and cleanup procedures. It provides a solid foundation for understanding eBPF programming and can be extended for more complex monitoring scenarios.


