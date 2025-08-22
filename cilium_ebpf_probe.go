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

// Configuration structure (must match eBPF program)
type Config struct {
	Verbose uint32
	Message [64]int8
}

// Event structure (must match eBPF program)
type Event struct {
	PID      uint32
	TGID     uint32
	Comm     [16]int8
	FuncName [16]int8
	Timestamp uint64
}

// Helper function to convert int8 array to string
func int8ArrayToString(arr []int8) string {
	var result []byte
	for _, b := range arr {
		if b == 0 {
			break
		}
		result = append(result, byte(b))
	}
	return string(result)
}

func main() {
	// Set resource limits for eBPF
	if err := rlimit.RemoveMemlock(); err != nil {
		log.Fatal(err)
	}

	// Load pre-compiled eBPF program using Cilium framework
	objs := bpfObjects{}
	if err := loadBpfObjects(&objs, nil); err != nil {
		log.Fatal("loading objects:", err)
	}
	defer objs.Close()

	// Open log file
	logFile, err := os.OpenFile("syscalls.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Printf("Warning: Could not open log file: %v", err)
	} else {
		defer logFile.Close()
	}

	// Set configuration in eBPF map using Cilium framework
	config := Config{
		Verbose: 1,
	}
	message := "Cilium Framework eBPF Probe"
	for i := 0; i < len(config.Message) && i < len(message); i++ {
		config.Message[i] = int8(message[i])
	}
	
	key := uint32(0)
	if err := objs.ConfigMap.Update(&key, &config, ebpf.UpdateAny); err != nil {
		log.Fatal("updating config map:", err)
	}

	// Attach kprobe for sys_read using Cilium framework
	kpRead, err := link.Kprobe("__x64_sys_read", objs.CiliumKprobeSysRead, nil)
	if err != nil {
		log.Fatal("attaching kprobe for sys_read:", err)
	}
	defer kpRead.Close()

	// Attach kprobe for sys_write using Cilium framework
	kpWrite, err := link.Kprobe("__x64_sys_write", objs.CiliumKprobeSysWrite, nil)
	if err != nil {
		log.Fatal("attaching kprobe for sys_write:", err)
	}
	defer kpWrite.Close()

	// Set up perf buffer for events using Cilium framework
	rd, err := perf.NewReader(objs.Events, 4096)
	if err != nil {
		log.Fatal("creating perf event reader:", err)
	}
	defer rd.Close()

	fmt.Println("Cilium Framework eBPF Probe")
	fmt.Println("============================")
	fmt.Printf("Verbose mode: %s\n", map[uint32]string{0: "disabled", 1: "enabled"}[config.Verbose])
	fmt.Printf("Message: %s\n", int8ArrayToString(config.Message[:]))
	fmt.Println("Press Ctrl+C to stop")
	fmt.Println()

	fmt.Println("eBPF program loaded and attached successfully!")
	fmt.Println("Monitoring sys_read and sys_write calls...")
	fmt.Println()

	// Handle events from eBPF program
	go func() {
		for {
			record, err := rd.Read()
			if err != nil {
				if err == perf.ErrClosed {
					return
				}
				log.Printf("reading from perf ring buffer: %v", err)
				continue
			}

			if record.LostSamples != 0 {
				log.Printf("perf ring buffer full, dropped %d samples", record.LostSamples)
				continue
			}

			var event Event
			if err := binary.Read(bytes.NewReader(record.RawSample), binary.LittleEndian, &event); err != nil {
				log.Printf("parsing event: %v", err)
				continue
			}

			// Print exact required message to screen
			funcName := int8ArrayToString(event.FuncName[:])
			fmt.Printf("hello %s was called\n", funcName)

			// Log to file
			if logFile != nil {
				timestamp := time.Now().Format("2006-01-02 15:04:05.000000000")
				comm := int8ArrayToString(event.Comm[:])
				logLine := fmt.Sprintf("[%s] hello %s was called by %s (PID: %d, TGID: %d)\n",
					timestamp, funcName, comm, event.PID, event.TGID)
				logFile.WriteString(logLine)
				logFile.Sync()
			}
		}
	}()

	// Wait for interrupt signal
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGINT, syscall.SIGTERM)
	<-sig

	fmt.Println("\nShutting down...")
}
