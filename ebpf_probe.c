#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

// Cilium-style configuration structure
struct config {
    __u32 verbose;
    char message[64];
} __attribute__((packed));

// Cilium-style event structure
struct event {
    __u32 pid;
    __u32 tgid;
    char comm[16];
    char func_name[16];
    __u64 timestamp;
} __attribute__((packed));

// Cilium-style map definitions with proper attributes
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, struct config);
} config_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(__u32));
} events SEC(".maps");

// Cilium-style kprobe for sys_read
SEC("kprobe/__x64_sys_read")
int cilium_kprobe_sys_read(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg;
    struct event e = {};
    
    // Cilium-style map lookup
    cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg) {
        return 0;
    }
    
    // Cilium-style data collection
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    
    // Set function name for sys_read
    __builtin_memcpy(e.func_name, "sys_read", 8);
    e.timestamp = bpf_ktime_get_ns();
    
    // Cilium-style event output
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));
    
    // Cilium-style verbose logging (optional)
    if (cfg->verbose) {
        bpf_printk("hello sys_read was called by %s (PID: %d)", e.comm, e.pid);
    }
    
    return 0;
}

// Cilium-style kprobe for sys_write
SEC("kprobe/__x64_sys_write")
int cilium_kprobe_sys_write(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg;
    struct event e = {};
    
    // Cilium-style map lookup
    cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg) {
        return 0;
    }
    
    // Cilium-style data collection
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    
    // Set function name for sys_write
    __builtin_memcpy(e.func_name, "sys_write", 9);
    e.timestamp = bpf_ktime_get_ns();
    
    // Cilium-style event output
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));
    
    // Cilium-style verbose logging (optional)
    if (cfg->verbose) {
        bpf_printk("hello sys_write was called by %s (PID: %d)", e.comm, e.pid);
    }
    
    return 0;
}

// Cilium-style license declaration
char _license[] SEC("license") = "GPL";
