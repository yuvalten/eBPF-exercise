# Quick Start - REAL Cilium Framework eBPF Solution

Get up and running in 5 minutes!

## 🚀 Quick Start Commands

```bash
# 1. Install dependencies
make install-deps

# 2. Build everything
make clean && make

# 3. Test the solution
make test

# 4. Run manually
sudo ./cilium_probe
```

## 📋 What You'll See

When you run the program, you'll see:
```
Cilium Framework eBPF Probe
============================
Verbose mode: enabled
Message: Cilium Framework eBPF Probe
Press Ctrl+C to stop

eBPF program loaded and attached successfully!
Monitoring sys_read and sys_write calls...

hello sys_read was called
hello sys_write was called
hello sys_read was called
...
```

## 🔍 Monitor in Real-Time

```bash
# Terminal 1: Run the probe
sudo ./cilium_probe

# Terminal 2: Watch the log file
tail -f syscalls.log

# Terminal 3: Generate some activity
cat /etc/passwd
echo "test" > /tmp/test.txt
```

## 🛠️ Customization

The current implementation includes:
- **Verbose mode**: Enabled by default
- **Custom message**: "Cilium Framework eBPF Probe"
- **Real-time monitoring**: Continuous syscall tracking
- **Logging**: All events saved to `syscalls.log`

## ✅ Verification

The solution is working correctly when you see:
- ✅ "hello sys_read was called" for read operations
- ✅ "hello sys_write was called" for write operations
- ✅ Events appearing in `syscalls.log`
- ✅ Real-time screen output (not from trace_pipe)

## 🎯 Requirements Met

This solution fulfills **ALL** requirements:
- ✅ Uses **REAL Cilium framework** (`github.com/cilium/ebpf`)
- ✅ Kprobes for sys_read/sys_write
- ✅ Screen output (no trace_pipe)
- ✅ Log file output
- ✅ Configuration passing from user space to eBPF
- ✅ Data transfer from eBPF to user space

## 🆘 Troubleshooting

```bash
# Check if eBPF is supported
ls /sys/kernel/btf/vmlinux

# Verify Go and Cilium dependencies
go version
go list -m github.com/cilium/ebpf

# Rebuild if needed
make clean && make
```

## 📚 Next Steps

- Read `CILIUM_FRAMEWORK.md` for framework details
- Check `REQUIREMENTS_COMPLIANCE.md` for full compliance
- Explore `INSTALL.md` for detailed setup
- Run `make help` for all available commands

**You're all set!** 🎉

The solution now uses the **REAL Cilium framework** with Go user space and automatic code generation!
