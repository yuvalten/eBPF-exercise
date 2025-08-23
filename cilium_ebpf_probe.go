package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/perf"
	"github.com/cilium/ebpf/rlimit"
)

//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -cc clang -cflags "-O2 -g" bpf ebpf_probe.c -- -I/usr/include/bpf

// Config structure defines the configuration passed to the eBPF program
// This structure must match exactly with the corresponding struct in the eBPF C code
type Config struct {
	Verbose uint32 // Verbose logging level (0=disabled, 1=enabled)
}

// Event structure defines the event data sent from eBPF program to user space
// This structure must match exactly with the corresponding struct in the eBPF C code
type Event struct {
	PID       uint32   // Process ID of the calling process
	TGID      uint32   // Thread Group ID (main process ID)
	Comm      [16]int8 // Process name/command (fixed 16-byte buffer)
	FuncName  [16]int8 // Function name that was called (fixed 16-byte buffer)
	Timestamp uint64   // Timestamp when the event occurred
}

// int8ArrayToString converts a fixed-size int8 array to a Go string
// This is necessary because eBPF programs use fixed-size char arrays
// The conversion stops at the first null byte (0) encountered
func int8ArrayToString(arr []int8) string {
	var result []byte
	for _, b := range arr {
		if b == 0 {
			break // Stop at null terminator
		}
		result = append(result, byte(b))
	}
	return string(result)
}

func main() {
	// Remove memory lock limits required for eBPF operations
	// eBPF programs need to lock memory pages which requires elevated privileges
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal("Failed to remove memory lock limit:", err)
	}

	// Load the pre-compiled eBPF program objects into memory
	// This loads the eBPF bytecode that was generated from the C source
	objs := bpfObjects{}
	if err := loadBpfObjects(&objs, nil); err != nil {
		log.Fatal("Failed to load eBPF objects:", err)
	}
	// Ensure eBPF objects are properly cleaned up when program exits
	defer objs.Close()

	// Open log file for persistent event logging
	// O_APPEND: Append to existing file, O_CREATE: Create if doesn't exist, O_WRONLY: Write-only mode
	logFile, err := os.OpenFile("syscalls.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Printf("Warning: Could not open log file: %v", err)
	} else {
		defer logFile.Close()
	}

	// Create configuration structure to pass to eBPF program
	config := Config{
		Verbose: 1, // Enable verbose logging in eBPF program
	}

	// Update the configuration map in the eBPF program
	// This passes the configuration from user space to kernel space
	key := uint32(0)
	if err := objs.ConfigMap.Update(&key, &config, ebpf.UpdateAny); err != nil {
		log.Fatal("Failed to update config map:", err)
	}

	// Attach kprobe (kernel probe) to the sys_read system call
	// kprobes allow us to hook into kernel functions and execute our eBPF code
	// __x64_sys_read is the x86_64 system call entry point for read()
	kpRead, err := link.Kprobe("__x64_sys_read", objs.CiliumKprobeSysRead, nil)
	if err != nil {
		log.Fatal("Failed to attach kprobe for sys_read:", err)
	}
	defer kpRead.Close()

	// Attach kprobe to the sys_write system call
	// __x64_sys_write is the x86_64 system call entry point for write()
	kpWrite, err := link.Kprobe("__x64_sys_write", objs.CiliumKprobeSysWrite, nil)
	if err != nil {
		log.Fatal("Failed to attach kprobe for sys_write:", err)
	}
	defer kpWrite.Close()

	// Create a perf event reader to receive data from the eBPF program
	// objs.Events is a perf event array map that the eBPF program writes to
	// 4096 is the ring buffer size in bytes for efficient kernel-user space communication
	rd, err := perf.NewReader(objs.Events, 4096)
	if err != nil {
		log.Fatal("Failed to create perf event reader:", err)
	}
	defer rd.Close()

	// Display program header and configuration information
	fmt.Println("Cilium Framework eBPF Probe")
	fmt.Println("============================")
	fmt.Printf("Verbose mode: %s\n", map[uint32]string{0: "disabled", 1: "enabled"}[config.Verbose])
	fmt.Println("Press Ctrl+C to stop")
	fmt.Println()

	fmt.Println("eBPF program loaded and attached successfully!")
	fmt.Println("Monitoring sys_read and sys_write calls...")
	fmt.Println()

	// Set up signal handling for graceful termination
	// Create a buffered channel to receive OS signals
	sigChan := make(chan os.Signal, 1)
	// Register to receive SIGINT (Ctrl+C) and SIGTERM signals
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Start event processing goroutine
	go func() {
		for {
			// Read event from the perf ring buffer
			// This blocks until an event is available or the reader is closed
			record, err := rd.Read()
			if err != nil {
				if err == perf.ErrClosed {
					// Reader was closed, exit gracefully
					fmt.Println("Perf reader closed, exiting event processing...")
					return
				}
				// For any other error, log it and exit to avoid infinite error loops
				log.Printf("Error reading from perf ring buffer: %v", err)
				return
			}

			// Check if any samples were lost due to buffer overflow
			if record.LostSamples != 0 {
				log.Printf("Perf ring buffer full, dropped %d samples", record.LostSamples)
				continue
			}

			// Parse the raw event data into our Event structure
			// binary.Read converts the raw bytes to the Event struct using little-endian encoding
			var event Event
			if err := binary.Read(bytes.NewReader(record.RawSample), binary.LittleEndian, &event); err != nil {
				log.Printf("Error parsing event data: %v", err)
				continue
			}

			// Extract function name from the event data
			funcName := int8ArrayToString(event.FuncName[:])

			// Display event information to console
			fmt.Printf("hello %s was called\n", funcName)

			// Log detailed event information to file if available
			if logFile != nil {
				timestamp := time.Now().Format("2006-01-02 15:04:05.000000000")
				comm := int8ArrayToString(event.Comm[:])
				logLine := fmt.Sprintf("[%s] hello %s was called by %s (PID: %d, TGID: %d)\n",
					timestamp, funcName, comm, event.PID, event.TGID)
				logFile.WriteString(logLine)
				logFile.Sync() // Ensure data is written to disk immediately
			}
		}
	}()

	// Wait for termination signal (Ctrl+C or SIGTERM)
	fmt.Println("Waiting for termination signal...")
	<-sigChan

	// Graceful shutdown sequence
	fmt.Println("\nReceived termination signal. Shutting down gracefully...")

	// Close the perf reader to stop receiving events
	// This will cause the event processing goroutine to exit
	rd.Close()

	// Give the goroutine a moment to finish processing
	time.Sleep(100 * time.Millisecond)

	fmt.Println("eBPF probe shutdown complete.")
}
