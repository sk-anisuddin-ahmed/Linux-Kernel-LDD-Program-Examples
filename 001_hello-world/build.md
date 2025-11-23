## Build & Run -
- bash
- make                          # Compile the module
- sudo insmod hello.ko       # Insert the module
- dmesg | tail                  # View kernel logs
- sudo rmmod hello           # Remove the module

## Useful Commands -
- bash
- lsmod                         # List loaded modules
- modinfo hello.ko           # Show module info
- cat /proc/devices             # View registered devices