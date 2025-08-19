# eBPF-exercise
Linux RT - Home Exercise @ Salt
Background
The purpose of this exercise is to evaluate your ability to work with new,
exciting low-level Linux technologies, design clear solutions, and write working
code. Don’t worry if you’re new to eBPF - learning and research are part of the
process.
The technology relevant for your role is eBPF. https://ebpf.io/
BPF (Berkeley Packet Filter) is a highly flexible and efficient virtual machine-like construct
within the Linux kernel, enabling bytecode execution at various hook points securely. It is
widely utilized across several Linux kernel subsystems, notably in networking, tracing,
and security (e.g., sandboxing).
Although BPF has been around since 1992, this document focuses on the enhanced
version, the Extended Berkeley Packet Filter (eBPF), which debuted in Kernel version 3.18.
This modern iteration has effectively replaced the original, now termed "classic" BPF
(cBPF). Historically, cBPF was recognized as the packet filtering language utilized by
tools like tcpdump. However, in contemporary Linux systems, only eBPF is executed.
When a classic BPF bytecode is loaded, it is automatically converted to eBPF format
within the kernel before execution.
Exercise
● Create an eBPF program
● eBPF programs should use the Cilium framework; eBPF code should be in C.
● eBPF should be generated as part of the build flow.
● eBPF program should be able to probe, using kprobes for both sys_read and
sys_write kernel functions.
The expected result:
Every time a call to sys_read and sys_write on the Linux machine it runs on, the eBPF
program will print “hello sys_read/sys_write was called”.
Output:
● Log file.

● screen user space print please - do not rely on
“/sys/kernel/debug/tracing/trace_pipe”.
● Please provide the eBPF as well as the user plan code.
● Please provide instructions for installation/running it.
● Be able to pass configuration information from the user plan code to the eBPF
program.
● Be able to pass some information from the eBPF program to the user plan code.
Once ready, we will set a meeting to discuss the implementation. We may ask some
questions regarding why it was developed as it was, as well as think together on
possible improvements.
There are numerous tutorials and videos out there that can help in getting started. Feel
free to use them. Using AI assistance is allowed and encouraged.
Best of Luck!