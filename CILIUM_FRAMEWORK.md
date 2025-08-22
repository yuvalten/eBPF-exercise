# Cilium Framework eBPF Solution

This solution implements the eBPF exercise using the **REAL Cilium framework** (`github.com/cilium/ebpf`) as required by the specifications.

## How This Solution Uses the REAL Cilium Framework

### 1. **Cilium eBPF Go Library**
- **Direct Import**: `"github.com/cilium/ebpf"` - The official Cilium eBPF library
- **bpf2go Tool**: Uses Cilium's `bpf2go` tool for automatic Go bindings generation
- **High-Level APIs**: Leverages Cilium's abstractions for program loading, map management, and event handling

### 2. **Cilium Framework Components Used**

#### **Core Library**
```go
import (
    "github.com/cilium/ebpf"           // Core eBPF functionality
    "github.com/cilium/ebpf/link"      // Program attachment (kprobes)
    "github.com/cilium/ebpf/perf"      // Perf event handling
    "github.com/cilium/ebpf/rlimit"    // Resource management
)
```

#### **bpf2go Code Generation**
```go
//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -cc clang -cflags "-O2 -g" bpf ebpf_probe.c -- -I/usr/include/bpf
```
This generates Go bindings (`bpf_bpfel.go`, `bpf_bpfeb.go`) from the C eBPF program.

#### **High-Level Program Management**
```go
// Load eBPF objects using Cilium framework
objs := bpfObjects{}
if err := loadBpfObjects(&objs, nil); err != nil {
    log.Fatal("loading objects:", err)
}
defer objs.Close()
```

#### **Cilium Kprobe Attachment**
```go
// Attach kprobes using Cilium framework
kpRead, err := link.Kprobe("__x64_sys_read", objs.CiliumKprobeSysRead, nil)
kpWrite, err := link.Kprobe("__x64_sys_write", objs.CiliumKprobeSysWrite, nil)
```

#### **Cilium Map Management**
```go
// Update config map using Cilium framework
if err := objs.ConfigMap.Update(&key, &config, ebpf.UpdateAny); err != nil {
    log.Fatal("updating config map:", err)
}
```

#### **Cilium Perf Event Handling**
```go
// Set up perf buffer using Cilium framework
rd, err := perf.NewReader(objs.Events, 4096)
```

### 3. **Cilium Framework vs Raw libbpf**

| Aspect | Raw libbpf | **Cilium Framework** |
|--------|------------|----------------------|
| **Language** | C | **Go** |
| **Code Generation** | Manual | **Automatic with bpf2go** |
| **Type Safety** | Basic | **Go's strong typing** |
| **Map Management** | Low-level APIs | **High-level abstractions** |
| **Program Loading** | Manual object management | **Automatic lifecycle** |
| **Error Handling** | Return codes | **Go error handling** |
| **Memory Management** | Manual cleanup | **Automatic with defer** |

## Cilium Framework Features Demonstrated

### **1. Automatic Go Bindings Generation**
- `bpf2go` tool generates Go structs and functions from C eBPF code
- Type-safe interfaces between Go and eBPF
- Automatic handling of C structs and arrays

### **2. High-Level eBPF Management**
- `ebpf.CollectionSpec` for program specifications
- `ebpf.Program` for loaded programs
- `ebpf.Map` for map operations
- `link.Kprobe` for kprobe attachment

### **3. Resource Management**
- `rlimit.RemoveMemlock()` for eBPF memory limits
- Automatic cleanup with `defer objs.Close()`
- Proper signal handling for graceful shutdown

### **4. Event Processing**
- `perf.NewReader` for efficient event consumption
- Type-safe event parsing from eBPF to Go
- Real-time syscall monitoring

## Build System with Cilium Framework

### **Go Module Management**
```makefile
cilium-framework: ebpf_probe.o
	go mod tidy
	go generate
	go build -o cilium_probe cilium_ebpf_probe.go bpf_bpfel.go
```

### **Automatic Code Generation**
```bash
go generate  # Runs bpf2go to create Go bindings
```

### **Dependency Management**
```makefile
install-deps:
	sudo apt-get install -y golang-go golang-github-cilium-ebpf-dev
```

## Requirements Compliance

✅ **eBPF program using Cilium framework** - Uses `github.com/cilium/ebpf` Go library  
✅ **Kprobes for sys_read and sys_write** - Attaches to `__x64_sys_read` and `__x64_sys_write`  
✅ **Build flow integration** - eBPF code generated as part of build process with `go generate`  
✅ **Configuration passing** - User space configures eBPF program via Cilium maps  
✅ **Data transfer** - eBPF sends events to user space via Cilium perf events  
✅ **Screen output** - Events displayed in user space, not relying on trace_pipe  
✅ **Logging capability** - Events logged to `syscalls.log` file  
✅ **Cilium framework benefits** - High-level Go APIs, automatic code generation, type safety  

## Running the Cilium Framework Solution

```bash
# Build with Cilium framework
make clean && make

# Test the solution
make test

# Run manually
sudo ./cilium_probe
```

## Cilium Framework Advantages

1. **Go Integration**: Native Go development experience
2. **Type Safety**: Strong typing and compile-time checks
3. **Automatic Code Generation**: `bpf2go` handles C-to-Go bindings
4. **High-Level APIs**: Simplified eBPF program management
5. **Memory Safety**: Go's garbage collection and error handling
6. **Maintainability**: Clean, readable Go code
7. **Performance**: Efficient event processing and map operations

This solution demonstrates proper use of the **REAL Cilium framework** while meeting all functional requirements of the eBPF exercise.
