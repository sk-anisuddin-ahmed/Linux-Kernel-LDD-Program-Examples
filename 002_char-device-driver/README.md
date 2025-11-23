# Character Device Driver

This project implements a Linux character device driver. It provides a simple interface for interacting with a character device, including operations for opening, reading, writing, and closing the device.

## Project Structure

- `src/char_device.c`: Implementation of the character device driver.
- `src/char_device.h`: Header file defining the interface for the character device driver.
- `src/device_operations.c`: Implements device operations such as open, release, read, and write functions.
- `Makefile`: Build instructions for compiling the character device driver.
- `Kconfig`: Configuration options for the kernel build system.
- `README.md`: Documentation for building, installing, and using the character device driver.

## Building the Driver

To build the character device driver, navigate to the project directory and run:

```
make
```

This will compile the source files and create a loadable kernel module.

## Installing the Driver

To install the driver, use the following command:

```
sudo insmod char_device.ko
```

Replace `char_device.ko` with the name of the compiled module if different.

## Using the Driver

Once the driver is installed, you can interact with the character device using standard file operations. You can open the device file (usually located in `/dev/`) and perform read and write operations.

## Unloading the Driver

To remove the driver from the kernel, use:

```
sudo rmmod char_device
```

## License

This project is licensed under the MIT License. See the LICENSE file for more details.