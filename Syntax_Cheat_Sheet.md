# Different Error Types

| Code        | Meaning                                | Typical Use Case                                |
|-------------|----------------------------------------|------------------------------------------------|
| -EPERM      | Operation not permitted                | Unauthorized action (e.g., locked sector)       |
| -EACCES     | Permission denied                      | Access rights violation                         |
| -ENOENT     | No such file or directory              | File/device doesn’t exist                       |
| -ESRCH      | No such process                        | Process lookup failed                           |
| -EINTR      | Interrupted system call                | Signal interrupted syscall                      |
| -ERESTARTSYS| Restart syscall if interrupted         | Mutex lock interrupted by signal                |
| -EIO        | I/O error                              | Hardware or filesystem error                    |
| -ENXIO      | No such device or address              | Invalid device access                           |
| -ENOMEM     | Out of memory                          | Allocation failure                              |
| -EFAULT     | Bad address                            | Invalid user pointer in `copy_from_user`        |
| -EINVAL     | Invalid argument                       | Bad parameter (e.g., out of range index)        |
| -ENOSPC     | No space left on device                | Write beyond buffer/device capacity             |
| -EBUSY      | Device or resource busy                | Resource locked/in use                          |
| -EEXIST     | File exists                            | Duplicate creation                              |
| -ENODEV     | No such device                         | Device not present                              |
| -ENOTTY     | Inappropriate ioctl for device         | Unsupported IOCTL command                       |
| -EPIPE      | Broken pipe                            | Write to closed pipe                            |
| -EAGAIN     | Retry, resource temporarily unavailable| Non-blocking operation would block        		 |
| -EOVERFLOW  | Value too large for defined data type  | Integer overflow                                |
| -ERANGE     | Math result not representable          | Out-of-range calculation                        |

---

# Header Usage Information - Kernel Space

### `#include <linux/kernel.h>`
- Provides core kernel facilities: logging (`printk`, `pr_info`, `pr_err`), basic macros, and common definitions.  
- Used for `pr_info()` and `pr_err()` messages in your driver.

### `#include <linux/module.h>`
- Required for building loadable kernel modules.  
- Defines macros like `MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`, and functions `module_init()` / `module_exit()`.  
- Used to declare your driver as a module and provide metadata.

### `#include <linux/kdev_t.h>`
- Provides macros for working with device numbers (`dev_t`).  
- Includes helpers like `MAJOR(dev_t)` and `MINOR(dev_t)` to extract major/minor numbers.  
- Used when printing major and minor in your init function.

### `#include <linux/fs.h>`
- Defines structures and functions for file operations in the kernel.  
- Provides `struct file_operations`, `struct inode`, and constants like `O_RDONLY`, `O_WRONLY`.  
- Used to implement `.read`, `.write`, `.open`, `.release`, `.unlocked_ioctl`.

### `#include <linux/device.h>`
- Provides APIs for creating device nodes in `/dev` via `class_create()` and `device_create()`.  
- Used to expose your character device to user space as `/dev/storageDevice`.

### `#include <linux/cdev.h>`
- Provides support for character devices (`struct cdev`).  
- Functions like `cdev_init()` and `cdev_add()` register your driver with the kernel.  
- Used to bind your `file_operations` to a device number.

### `#include <linux/uaccess.h>`
- Provides safe functions for copying data between user space and kernel space.  
- Functions: `copy_to_user()`, `copy_from_user()`.  
- Used in `.read`, `.write`, and IOCTL handlers to transfer data.

### `#include <linux/mutex.h>`
- Provides mutual exclusion locks (`struct mutex`).  
- Functions: `mutex_init()`, `mutex_lock()`, `mutex_unlock()`, `mutex_lock_interruptible()`.  
- Used to protect `storage_buffer` and `sector_lock_state` against concurrent access.

# Header Usage Information - User Space

### `<unistd.h>`
- Provides access to the **POSIX API**.
- Contains system call wrappers like:
  - `read()`
  - `write()`
  - `close()`
  - `lseek()`
- Used for basic file I/O operations.

### `<fcntl.h>`
- Defines **file control options**.
- Contains constants and functions for:
  - `open()` → open files
  - `O_RDONLY`, `O_WRONLY`, `O_RDWR` → file access modes
  - `O_CREAT`, `O_TRUNC` → file creation/truncation flags
- Used to set file descriptors and control file behavior.

### `<sys/ioctl.h>`
- Provides the **ioctl() system call** interface.
- Used to send control commands to device drivers.

```c
int fd = open("/dev/mydevice", O_RDWR);
ioctl(fd, IOCTL_COMMAND, &arg);
close(fd);
```

# Linux Kernel Module Parameters

## Overview
Linux kernel modules can expose tunable parameters to user space. These parameters can be set:
- At module load time (`insmod mymodule.ko param=value`)
- At runtime via sysfs (`/sys/module/mymodule/parameters/param`)

---

## Module Parameter Macros
```c
static int myint = 0;
module_param(myint, int, 0644);

```c
static unsigned int myuint = 10;
module_param(myuint, uint, 0644);

```c
static long mylong = -5;
module_param(mylong, long, 0644);
```

```c
static unsigned long myulong = 50;
module_param(myulong, ulong, 0644);
```

```c
static short myshort = 2;
module_param(myshort, short, 0644);
```

```c
static unsigned short myushort = 4;
module_param(myushort, ushort, 0644);
```

```c
static char *mystring = "hello";
module_param(mystring, charp, 0644);
```

```c
static bool myflag = true;
module_param(myflag, bool, 0644);
```

```c
static bool debug = false;
module_param(debug, invbool, 0644);
```

```c
static int myarr[3] = {1, 2, 3};
static int count;
module_param_array(myarr, int, &count, 0644);
```

```c
static unsigned int vals[4] = {10, 20, 30, 40};
static int vals_count;
module_param_array(vals, uint, &vals_count, 0644);
```

```c
static char *names[3] = {"a", "b", "c"};
static int name_count;
module_param_array(names, charp, &name_count, 0644);
```

```c
static int value = 42;
module_param_named(my_param, value, int, 0644);
```

---

#File Permission

`0644` is a common Unix/Linux file permission setting
- **Owner:** Read and Write (`rw-`)
- **Group:** Read only (`r--`)
- **Others:** Read only (`r--`)

Symbolically:
## Octal Breakdown
- **6 (Owner):** `4 + 2 = read + write`
- **4 (Group):** `read`
- **4 (Others):** `read`

The leading `0` indicates no special bits (setuid, setgid, sticky).

```bash
chmod 0644 filename
```

# IOCTL Commands Overview

IOCTL (Input/Output Control) provides a way to send control commands to device drivers.  
Each command is defined with macros like `_IOW`, `_IOR`, `_IOWR`.

## Types of IOCTL Macros
- **_IOW** → Write: Pass data *from user space to kernel*  
- **_IOR** → Read: Get data *from kernel to user space*  
- **_IOWR** → Read/Write: Both directions

## IOCTL Usage
```c
#define IOCTL_LOCK_SECTOR    	_IOW('L', 0x1, int)

int fd = open("/dev/mydevice", O_RDWR);
int sector = 10;
ioctl(fd, IOCTL_LOCK_SECTOR, &sector);

close(fd);
```

# Storage Driver Structures Explained

In Linux device driver development, these static structures are commonly used to register and manage a character device.

## 📦 `struct class *storage_class`
- Represents a **device class** in sysfs.
- Groups devices of the same type under `/sys/class/`.
- Used with `class_create()` and `class_destroy()`.

## 📦 `struct device *storage_device`
- Represents the actual **device object**.
- Created with `device_create()`.
- Appears under `/dev/` as the device file (e.g., `/dev/storage`).

## 🔢 `dev_t storage_dev_number`
- Holds the **device number** (major + minor).
- Allocated with `alloc_chrdev_region()` or `register_chrdev_region()`.
- Identifies the device uniquely in the kernel.

## ⚙️ `struct cdev storage_cdev`
- Represents the **character device structure**.
- Initialized with `cdev_init()` and added with `cdev_add()`.
- Links file operations (open, read, write, ioctl) to the device.

## 🛠️ Typical Usage Flow
1. Allocate a device number (`alloc_chrdev_region` → `storage_dev_number`).
2. Initialize and add the character device (`cdev_init` → `storage_cdev`).
3. Create a device class (`class_create` → `storage_class`).
4. Create the device node (`device_create` → `storage_device`).
5. Device file appears in `/dev/` for user-space access.

## 📖 Summary
- **`storage_class`** → groups devices in sysfs  
- **`storage_device`** → represents the device in `/dev/`  
- **`storage_dev_number`** → unique identifier (major/minor)  
- **`storage_cdev`** → character device structure linking operations

# Synchronization in Linux Kernel

```c
static DEFINE_MUTEX(storage_mutex);

mutex_lock(&storage_mutex);
mutex_unlock(&storage_mutex);
```

```c
struct semaphore sem_t;
sema_init(&sem_t, 1);
sema_init(&sem_t, N);
down(&sem_t);
down_interruptible(&sem_t);
up(&sem_t);
```

# Using `copy_from_user` and `copy_to_user` in Linux Kernel

```c
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
char kbuf[100];
if (copy_from_user(kbuf, user_buf, sizeof(kbuf))) 
{
    return -EFAULT; // error if not all bytes copied
}
```

# Linux Kernel Module Metadata

## 🔧 Module Entry and Exit

### `module_init(storage_driver_init);`
- Defines the **entry point** of the module.
- When the module is loaded (`insmod`), the function `storage_driver_init()` is called.
- Typically used to:
  - Register devices
  - Allocate resources
  - Initialize data structures

### `module_exit(storage_driver_exit);`
- Defines the **exit point** of the module.
- When the module is unloaded (`rmmod`), the function `storage_driver_exit()` is called.
- Typically used to:
  - Unregister devices
  - Free resources
  - Clean up memory

---

## Module Information Macros

### `MODULE_LICENSE("GPL");`
- Declares the license type (here, **GPL**).
- Required for kernel modules to avoid "tainting" the kernel.
- Ensures compliance with open-source licensing.

### `MODULE_AUTHOR("SK AHMED");`
- Specifies the author of the module.
- Useful for documentation and attribution.

### `MODULE_DESCRIPTION("Sector-based 4KB storage driver with lock/unlock/erase support");`
- Provides a short description of the module.
- Helps identify the purpose of the driver when listing modules (`modinfo`).


# Basic File Operations in Kernel Space

## Overview
In Linux kernel modules, you can perform file I/O using kernel APIs like `filp_open`, `kernel_write`, and `filp_close`.  
These are used when the driver needs to read/write files directly from kernel space.

## Common Functions
```c
struct file *filp;
filp = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
```
- Opens a file in kernel space
- Parameters:
- path → file path (string)
- flags → e.g. O_WRONLY, O_CREAT, O_TRUNC
- mode → file permission (e.g. 0644)
- Returns a struct file * or error pointer.

```c
kernel_write()
ssize_t rc = kernel_write(filp, buffer, size, &pos);
```

- Writes data to the file
- Parameters:
- filp → file pointer
- buffer → kernel buffer containing data
- size → number of bytes to write
- pos → file offset (updated after write)
- Returns number of bytes written or negative error code.

```c
filp_close()
filp_close(filp, NULL);
```

- Closes the file.
- Frees resources associated with the file pointer.

# File Operations in Linux Kernel Driver

## 📌 Overview
The `struct file_operations` defines how user-space interacts with a character device.  
Each field points to a function in the driver that handles a specific operation.

## 🔧 Fields in `storage_fops`

```c
static const struct file_operations storage_fops = {
    .owner          = THIS_MODULE,
    .read           = storage_read,
    .write          = storage_write,
    .open           = storage_open,
    .release        = storage_release,
    .unlocked_ioctl = storage_ioctl,
    .llseek         = default_llseek,
};
```

# Device Creation Flow in Linux Character Driver

## __Init

```c
ret = alloc_chrdev_region(&storage_dev_number, 0, 1, "storageDevice");
```
- Reserves a major/minor number for the device.
- Initialize cdev

```c
cdev_init(&storage_cdev, &storage_fops);
storage_cdev.owner = THIS_MODULE;
```
- Links file operations (storage_fops) to the character device.
- Add cdev to kernel

```c
ret = cdev_add(&storage_cdev, storage_dev_number, 1);
```
- Registers the character device with the kernel.
- Create device class

```c
storage_class = class_create(THIS_MODULE, "storage_class");
```
- Creates a class entry under /sys/class/.
- Create device node

```c
storage_device = device_create(storage_class, NULL, storage_dev_number, NULL, "storageDevice");
```
- Creates /dev/storageDevice for user-space access.

## __Exit

```c
device_destroy(storage_class, storage_dev_number);
class_destroy(storage_class);
cdev_del(&storage_cdev);
unregister_chrdev_region(storage_dev_number, 1);
````
- Removes device node, class, cdev, and frees device number.

# Kernel Logging: `pr_info` vs `printk`

## `printk`
- The traditional logging function in the Linux kernel.
- Syntax:

```c
printk(KERN_INFO "Message: %d\n", value);
```

- `KERN_EMERG`  
  - Emergency: system is unusable  
  - Highest priority

- `KERN_ALERT`  
  - Action must be taken immediately

- `KERN_CRIT`  
  - Critical conditions (serious hardware/software errors)

- `KERN_ERR`  
  - Error conditions

- `KERN_WARNING`  
  - Warning conditions

- `KERN_NOTICE`  
  - Normal but significant condition

- `KERN_INFO`  
  - Informational messages

- `KERN_DEBUG`  
  - Debug-level messages (lowest priority)

## `pr_info`
- A convenience macro built on top of printk.
- Automatically uses the INFO log level.

```c
pr_info("Message: %d\n", value);
```

- Cleaner and shorter than printk.

## `other print`
```c
pr_err() → error messages
pr_warn() → warnings
pr_debug() → debug logs
pr_info() → informational logs
```

# Makefile Documentation for Storage Driver

```make
obj-m += storage_kernel.o storage_mirror_kernel.o
```
- Defines kernel modules to be built
- obj-m means "object as module"

```make
.PHONY: all kernel user clean
```
- Declares targets that are not actual files.
- Ensures commands run even if files with the same name exist.

```make
all: clean kernel user
```
- Runs clean, then builds both kernel and user parts.
- Ensures a fresh build each time.

```make
kernel:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```
- Builds kernel modules against the current kernel source.
- -C → change directory to kernel build tree.
- M=$(PWD) → use current directory as module source.

```make
user:
    gcc storage_user.c -o storage_user
```
- Compiles user-space program storage_user.c.
- Produces executable storage_user.

```make
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
    rm -f storage_user *.o *.out
```
- Cleans kernel build artifacts.
- Removes user program and temporary files.