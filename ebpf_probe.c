#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

// Configuration structure
struct config {
    int verbose;
    char message[64];
};

// Event structure
struct event {
    __u32 pid;
    __u32 tgid;
    char comm[16];
    char func_name[16];
    __u64 timestamp;
};

// Configuration map
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, struct config);
} config_map SEC(".maps");

// Events map
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(__u32));
} events SEC(".maps");

// Kprobe for sys_read
SEC("kprobe/__x64_sys_read")
int kprobe_sys_read(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg)
        return 0;

    struct event e = {};
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    __builtin_memcpy(e.func_name, "sys_read", 8);
    e.timestamp = bpf_ktime_get_ns();

    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));

    if (cfg->verbose) {
        bpf_printk("hello sys_read was called by %s (PID: %d)", e.comm, e.pid);
    }
    return 0;
}

// Kprobe for sys_write
SEC("kprobe/__x64_sys_write")
int kprobe_sys_write(struct pt_regs *ctx)
{
    __u32 key = 0;
    struct config *cfg = bpf_map_lookup_elem(&config_map, &key);
    if (!cfg)
        return 0;

    struct event e = {};
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    e.pid = (__u32)pid_tgid;
    e.tgid = (__u32)(pid_tgid >> 32);
    bpf_get_current_comm(&e.comm, sizeof(e.comm));
    __builtin_memcpy(e.func_name, "sys_write", 9);
    e.timestamp = bpf_ktime_get_ns();

    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &e, sizeof(e));

    if (cfg->verbose) {
        bpf_printk("hello sys_write was called by %s (PID: %d)", e.comm, e.pid);
    }
    return 0;
}

char _license[] SEC("license") = "GPL";
