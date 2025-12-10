# Hello World Character Device Driver

A simple Linux kernel module that implements a basic character device driver for learning purposes.

## Features

- Character device registration and management
- Basic file operations (open, read, write, release)
- Automatic device node creation via sysfs
- User-kernel space data transfer
- Proper error handling and cleanup

## Files

- `char_device.c` - Main driver source code
- `Makefile` - Build configuration
- `Kconfig` - Kernel configuration options
- `README.md` - This documentation

## Building

```bash
make
```

## Installation

```bash
# Load the module
make install

# Or manually
sudo insmod char_device.ko
```

## Usage

```bash
# Test the device
make test

# Or manually create device node and test
sudo mknod /dev/hello_char c $(cat /proc/devices | grep hello_char | cut -d' ' -f1) 0

# Write to device
echo "Hello World!" | sudo tee /dev/hello_char

# Read from device
sudo cat /dev/hello_char

# Clean up
sudo rm /dev/hello_char
```

## Uninstallation

```bash
# Remove the module
make uninstall

# Or manually
sudo rmmod char_device
```

## Monitoring

```bash
# Check kernel messages
dmesg | tail

# Check loaded modules
lsmod | grep char_device

# Check device registration
cat /proc/devices | grep hello_char
```

## Learning Objectives

This driver demonstrates:

1. Kernel module initialization and cleanup
2. Character device registration with the kernel
3. Implementation of file operations structure
4. Device class and device creation for automatic `/dev` entry
5. Safe data transfer between user and kernel space
6. Proper error handling and resource cleanup

## Notes

- This is a educational driver - not for production use
- Requires root privileges to load/unload
- Compatible with Linux kernel 4.0+
