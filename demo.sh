#!/bin/bash

echo "eBPF System Call Monitor Demo"
echo "============================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This demo must be run as root (sudo)${NC}"
    exit 1
fi

# Check if user_program exists
if [ ! -f "./user_program" ]; then
    echo -e "${RED}Error: user_program not found. Please build the project first with 'make'${NC}"
    exit 1
fi

echo -e "${GREEN}Starting eBPF system call monitor...${NC}"
echo "This will monitor ALL sys_read and sys_write calls on your system"
echo "Press Ctrl+C to stop the monitor"
echo ""

# Function to cleanup on exit
cleanup() {
    echo -e "\n${YELLOW}Cleaning up...${NC}"
    if [ ! -z "$PROBE_PID" ]; then
        kill $PROBE_PID 2>/dev/null
        wait $PROBE_PID 2>/dev/null
    fi
    echo -e "${GREEN}Demo completed!${NC}"
    exit 0
}

# Set trap for cleanup
trap cleanup INT TERM

# Start the eBPF probe in background with verbose mode
echo -e "${BLUE}Starting probe in background...${NC}"
./user_program -v -m "Live system call monitor" &
PROBE_PID=$!

# Wait for probe to start
sleep 3

if ! kill -0 $PROBE_PID 2>/dev/null; then
    echo -e "${RED}Failed to start eBPF probe${NC}"
    exit 1
fi

echo -e "${GREEN}Probe started successfully with PID: $PROBE_PID${NC}"
echo -e "${YELLOW}Now generating some system activity to demonstrate monitoring...${NC}"
echo ""

# Generate some activity in the background
(
    sleep 2
    echo -e "${BLUE}Generating sys_read activity...${NC}"
    cat /proc/version > /dev/null
    cat /proc/cpuinfo | head -3 > /dev/null
    
    sleep 2
    echo -e "${BLUE}Generating sys_write activity...${NC}"
    echo "Test data for eBPF demo" > /tmp/ebpf_demo.txt
    
    sleep 2
    echo -e "${BLUE}Generating more sys_read activity...${NC}"
    cat /tmp/ebpf_demo.txt > /dev/null
    ls -la /tmp/ | head -3 > /dev/null
    
    sleep 2
    echo -e "${BLUE}Cleaning up demo files...${NC}"
    rm -f /tmp/ebpf_demo.txt
    
    echo -e "${GREEN}Demo activity completed!${NC}"
    echo -e "${YELLOW}The eBPF probe should have captured all these system calls above.${NC}"
    echo -e "${YELLOW}Press Ctrl+C to stop monitoring.${NC}"
) &

echo -e "${GREEN}Demo is running!${NC}"
echo -e "${YELLOW}Watch the output above to see system calls being monitored in real-time.${NC}"
echo -e "${YELLOW}The probe will continue running until you press Ctrl+C.${NC}"
echo ""

# Keep the main script running
while kill -0 $PROBE_PID 2>/dev/null; do
    sleep 1
done

cleanup


