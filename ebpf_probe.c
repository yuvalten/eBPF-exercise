#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

// configuration map (index 0): 'verbose' flag
struct config {
    __u32 verbose;
} __attribute__((packed));

// Event record map sent to user space
struct event {
    __u32 pid;
    __u32 tgid;
    char comm[16];
    char func_name[16];
    __u64 timestamp;
} __attribute__((packed));

// Maps: config (BPF_ARRAY,1) and events (PERF_EVENT_ARRAY)
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

// kprobe: __x64_sys_read
SEC("kprobe/__x64_sys_read")
int cilium_kprobe_sys_read(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg;
    struct event e = {};
    
    // Read config (key=0)
    cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg) {
        return 0;
    }
    
    // Fill event (pid/tgid/comm/timestamp)
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    
    // func_name = "sys_read"
    __builtin_memcpy(e.func_name, "sys_read", 8);
    e.timestamp = bpf_ktime_get_ns();
    
    // Submit to perf event buffer
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));
    
    // Optional printk when verbose
    if (cfg->verbose) {
        bpf_printk("hello sys_read was called by %s (PID: %d)", e.comm, e.pid);
    }
    
    return 0;
}

// kprobe: __x64_sys_write
SEC("kprobe/__x64_sys_write")
int cilium_kprobe_sys_write(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg;
    struct event e = {};
    
    // Read config (key=0)
    cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg) {
        return 0;
    }
    
    // Fill event (pid/tgid/comm/timestamp)
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    
    // func_name = "sys_write"
    __builtin_memcpy(e.func_name, "sys_write", 9);
    e.timestamp = bpf_ktime_get_ns();
    
    // Submit to perf event buffer
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));
    
    // Optional printk when verbose
    if (cfg->verbose) {
        bpf_printk("hello sys_write was called by %s (PID: %d)", e.comm, e.pid);
    }
    
    return 0;
}

// License
char _license[] SEC("license") = "GPL";
