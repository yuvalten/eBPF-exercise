#!/bin/bash

echo "eBPF Probe Test Script"
echo "======================"
echo ""

# Check if user_program exists
if [ ! -f "./user_program" ]; then
    echo "Error: user_program not found. Please build the project first with 'make'"
    exit 1
fi

# Check if running as root (required for eBPF)
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (sudo) for eBPF to work"
    exit 1
fi

echo "Starting eBPF probe in background..."
echo "The probe will monitor all sys_read and sys_write calls on the system"
echo ""

# Start the eBPF probe in background
./user_program -v -m "Test probe message" &
PROBE_PID=$!

# Wait a moment for the probe to start
sleep 2

echo "Probe started with PID: $PROBE_PID"
echo "Now generating some sys_read/sys_write activity..."
echo ""

# Generate some activity that will trigger sys_read/sys_write
echo "1. Reading from /proc/version (triggers sys_read)..."
cat /proc/version > /dev/null

echo "2. Writing to a temporary file (triggers sys_write)..."
echo "Test data" > /tmp/test_probe.txt

echo "3. Reading the temporary file (triggers sys_read)..."
cat /tmp/test_probe.txt > /dev/null

echo "4. Running 'ls' command (triggers multiple sys_read/sys_write)..."
ls -la /tmp/ | head -5

echo "5. Checking system load (triggers sys_read)..."
uptime

echo ""
echo "Cleaning up..."
rm -f /tmp/test_probe.txt

echo "Stopping eBPF probe..."
kill $PROBE_PID

echo ""
echo "Test completed! Check the output above to see the eBPF probe in action."
echo "The probe should have printed messages for each sys_read and sys_write call."


