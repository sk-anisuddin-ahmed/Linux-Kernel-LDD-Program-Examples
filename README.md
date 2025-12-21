# Linux Kernel Linux Device Drivers (LDD) Examples

This workspace contains various examples of Linux Kernel Device Drivers, primarily character drivers, demonstrating different concepts in Linux Kernel Module programming.

## Project Structure

### 000_hello_world/

A basic "Hello World" kernel module that prints messages on load and unload, including the current timestamp.

- `hello.c`: Kernel module source
- `Makefile`: Build script for kernel module

### 000_bbb_drivers/

BeagleBone Black specific drivers setup.

#### 000_hello/

Another hello world example, possibly for BBB environment.

### 001_temp_log/

A character device driver for temperature logging with ioctl support.

- `driver.c`: Main kernel driver
- `helper.c`: Helper functions (likely temperature simulation)
- `user_app.c`: User space application to interact with the driver
- `Makefile`: Build script

Features:

- Read temperature values
- IOCTL commands for threshold checking
- Module parameters for high/low thresholds

### 002_char_keypad_driver/

Character driver for keypad input.

- `key_kernel.c`: Kernel driver
- `key_user.c`: User space app
- `keypad_helper_kernel.c`: Helper functions
- `queue_kernel.c`, `queue_kernel.h`: Queue implementation
- `Makefile`: Build script

### 003_char_block_storage_device/

Block storage device driver example.

- `storage_kernel.c`: Kernel driver
- `storage_mirror_kernel.c`: Mirror storage implementation
- `storage_user.c`: User space interface
- `Makefile`: Build script

### 004_temp_sens_atomic/

Temperature sensor driver using atomic operations.

- `temp_kernel.c`: Kernel driver
- `temp_user.c`: User space app
- `Makefile`: Build script

### 005_atomic_light/

Light control driver using atomic operations.

- `light_kernel.c`: Kernel driver
- `light_user.c`: User space app
- `Makefile`: Build script

### __Char_Driver_Misc/

Miscellaneous character driver examples and utilities.

- `hexdump.c`: Hex dump utility
- `init_exit_data.c`: Init/exit data handling
- `insmod_flow.md`: Documentation on insmod flow
- `kThread.c`: Kernel thread example
- `misc.c`: Misc driver code
- `panic.c`: Panic handling
- `Makefile`: Build script
- `Topics/`: Additional documentation

## Syntax Cheat Sheet

`Syntax_Cheat_Sheet.md` contains a comprehensive reference for:

- Error codes
- Header file usage (kernel and user space)
- Module parameters
- File permissions
- IOCTL commands
- And more kernel programming concepts

## Setup

Refer to `Setup/bbb-usb-internet-and-ko-build.md` for BeagleBone Black setup instructions.

## Building and Running

Each project directory contains a Makefile. To build:

```bash
cd <project_dir>
make
```

To load a module:

```bash
sudo insmod <module>.ko
```

To unload:

```bash
sudo rmmod <module>
```

Check dmesg for kernel messages:

```bash
dmesg | tail
```

## Notes

- These are educational examples for learning Linux Kernel Device Drivers
- Ensure you have kernel headers installed for building
- Some examples may require specific hardware (like BBB)
- Always test in a safe environment as kernel modules can crash the system
