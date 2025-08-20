#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/resource.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>

// Configuration structure (must match eBPF program)
struct config {
    int verbose;
    char message[64];
};

// Event structure (must match eBPF program)
struct event {
    __u32 pid;
    __u32 tgid;
    char comm[16];
    char func_name[16];
    __u64 timestamp;
};

static volatile bool running = true;
static FILE *log_fp = NULL;

// Signal handler for graceful shutdown
static void sig_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = false;
}

// Event handler for perf events from eBPF
static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
    const struct event *e = data;

    // Print exact required message to screen
    printf("hello %s was called\n", e->func_name);

    // Append to log file
    if (log_fp) {
        fprintf(log_fp, "hello %s was called\n", e->func_name);
        fflush(log_fp);
    }
}

// Print usage information
static void print_usage(const char *prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -v, --verbose    Enable verbose mode\n");
    printf("  -m, --message    Custom message (max 63 chars)\n");
    printf("  -h, --help       Show this help message\n");
    printf("\nExample:\n");
    printf("  %s -v -m \"Custom probe message\"\n", prog_name);
}

int main(int argc, char *argv[]) {
    struct bpf_object *obj = NULL;
    struct bpf_program *prog = NULL;
    struct bpf_link *link_read = NULL, *link_write = NULL;
    struct perf_buffer *pb = NULL;
    struct config cfg = {0};
    int err = 0, opt;
    char *message = (char *)"Default probe message";
    const char *log_path = "syscalls.log";
    
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "vhm:")) != -1) {
        switch (opt) {
        case 'v':
            cfg.verbose = 1;
            break;
        case 'm':
            message = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Truncate message if too long
    strncpy(cfg.message, message, sizeof(cfg.message) - 1);
    cfg.message[sizeof(cfg.message) - 1] = '\0';
    
    printf("eBPF Probe Program\n");
    printf("==================\n");
    printf("Verbose mode: %s\n", cfg.verbose ? "enabled" : "disabled");
    printf("Message: %s\n", cfg.message);
    printf("Press Ctrl+C to stop\n\n");

    // Open log file (append mode)
    log_fp = fopen(log_path, "a");
    if (!log_fp) {
        fprintf(stderr, "Failed to open log file %s: %s\n", log_path, strerror(errno));
        // Not fatal; continue without file logging
    }
    
    // Set up signal handlers
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    
    // Increase resource limits
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    if (setrlimit(RLIMIT_MEMLOCK, &r) != 0) {
        fprintf(stderr, "Failed to set RLIMIT_MEMLOCK: %s\n", strerror(errno));
        err = 1;
        goto cleanup;
    }
    
    // Load eBPF object
    obj = bpf_object__open("ebpf_probe.o");
    if (libbpf_get_error(obj)) {
        fprintf(stderr, "Failed to open eBPF object: %s\n", strerror(errno));
        err = 1;
        goto cleanup;
    }
    
    // Load eBPF program
    err = bpf_object__load(obj);
    if (err) {
        fprintf(stderr, "Failed to load eBPF object: %s\n", strerror(errno));
        goto cleanup;
    }
    
    // Get the program
    prog = bpf_object__find_program_by_name(obj, "kprobe_sys_read");
    if (!prog) {
        fprintf(stderr, "Failed to find kprobe_sys_read program\n");
        err = -1;
        goto cleanup;
    }
    
    // Attach kprobe for sys_read
    link_read = bpf_program__attach(prog);
    if (libbpf_get_error(link_read)) {
        fprintf(stderr, "Failed to attach kprobe for sys_read: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    // Get and attach kprobe for sys_write
    prog = bpf_object__find_program_by_name(obj, "kprobe_sys_write");
    if (!prog) {
        fprintf(stderr, "Failed to find kprobe_sys_write program\n");
        err = -1;
        goto cleanup;
    }
    
    link_write = bpf_program__attach(prog);
    if (libbpf_get_error(link_write)) {
        fprintf(stderr, "Failed to attach kprobe for sys_write: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    // Set configuration in eBPF map
    int config_map_fd = bpf_object__find_map_fd_by_name(obj, "config_map");
    if (config_map_fd < 0) {
        fprintf(stderr, "Failed to find config_map: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    __u32 key = 0;
    if (bpf_map_update_elem(config_map_fd, &key, &cfg, BPF_ANY) != 0) {
        fprintf(stderr, "Failed to update config map: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    // Set up perf buffer for events
    int events_map_fd = bpf_object__find_map_fd_by_name(obj, "events");
    if (events_map_fd < 0) {
        fprintf(stderr, "Failed to find events map: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    pb = perf_buffer__new(events_map_fd, 64, handle_event, NULL, NULL, NULL);
    if (libbpf_get_error(pb)) {
        fprintf(stderr, "Failed to create perf buffer: %s\n", strerror(errno));
        err = -1;
        goto cleanup;
    }
    
    printf("eBPF program loaded and attached successfully!\n");
    printf("Monitoring sys_read and sys_write calls...\n\n");
    
    // Main event loop
    while (running) {
        err = perf_buffer__poll(pb, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "Error polling perf buffer: %s\n", strerror(-err));
            break;
        }
    }
    
    printf("\nShutting down...\n");
    
cleanup:
    if (pb) perf_buffer__free(pb);
    if (link_read) bpf_link__destroy(link_read);
    if (link_write) bpf_link__destroy(link_write);
    if (obj) bpf_object__close(obj);
    if (log_fp) { fclose(log_fp); log_fp = NULL; }
    
    return err != 0;
}
