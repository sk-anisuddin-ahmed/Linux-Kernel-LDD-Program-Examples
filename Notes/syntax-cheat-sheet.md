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
```

```c
static unsigned int myuint = 10;
module_param(myuint, uint, 0644);
```

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

## `struct class *storage_class`
- Represents a **device class** in sysfs.
- Groups devices of the same type under `/sys/class/`.
- Used with `class_create()` and `class_destroy()`.

## `struct device *storage_device`
- Represents the actual **device object**.
- Created with `device_create()`.
- Appears under `/dev/` as the device file (e.g., `/dev/storage`).

## `dev_t storage_dev_number`
- Holds the **device number** (major + minor).
- Allocated with `alloc_chrdev_region()` or `register_chrdev_region()`.
- Identifies the device uniquely in the kernel.

## `struct cdev storage_cdev`
- Represents the **character device structure**.
- Initialized with `cdev_init()` and added with `cdev_add()`.
- Links file operations (open, read, write, ioctl) to the device.

## Typical Usage Flow
1. Allocate a device number (`alloc_chrdev_region` → `storage_dev_number`).
2. Initialize and add the character device (`cdev_init` → `storage_cdev`).
3. Create a device class (`class_create` → `storage_class`).
4. Create the device node (`device_create` → `storage_device`).
5. Device file appears in `/dev/` for user-space access.

## Summary
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

## Overview
The `struct file_operations` defines how user-space interacts with a character device.
Each field points to a function in the driver that handles a specific operation.

## Fields in `storage_fops`

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

# Linux Shell Commands

## Help & shell basics
man ls                  # Manual page (help) for a command
help cd                 # Built-in shell help
type ls                 # How a command is resolved
which ls                # Path to an executable
whereis ls              # Locate binary/source/man
echo "text"             # Print text
printf "%s\n" "text"    # Safer echo
history                 # Show command history
alias ll='ls -la'       # Create a command alias
unalias ll              # Remove alias
env                     # Show environment
export VAR=value        # Set env var (current shell)
source ~/.bashrc        # Reload shell config

## Navigation & files
pwd                     # Current directory
ls -lAh                 # List files (long, all, human)
cd /path                # Change directory
cd -                    # Switch to previous directory
mkdir -p dir/subdir     # Create nested dirs
touch file.txt          # Create empty file
cp file1 file2          # Copy file
mv old new              # Move/rename
rm file.txt             # Remove file
rm -rf dir/             # Force remove directory
ln -s target link       # Symlink
stat file.txt           # File details
file file.txt           # Detect file type
du -sh dir/             # Dir size (summary)
df -h                   # Disk usage (human)
find . -name "*.log"    # Find files by name
grep -r "text" .        # Search text recursively
sed -n '1,10p' file     # Print lines 1–10
awk '{print $1}' file   # Print first column
sort file | uniq -c     # Unique lines with counts
cut -d, -f2 file.csv    # Extract column 2 (CSV)
paste f1 f2             # Merge files line-wise
tr '[:lower:]' '[:upper:]' < f # Transform chars
wc -l file.txt          # Count lines
diff a.txt b.txt        # Compare files
patch < change.diff     # Apply patch

## Viewing & monitoring
cat file.txt            # Show file
tac file.txt            # Reverse cat
less file.txt           # Scroll view
head -n 20 file.txt     # First 20 lines
tail -n 50 file.txt     # Last 50 lines
tail -f log.txt         # Follow updates
watch -n 2 'df -h'      # Repeat a command
time ls                 # Measure command time
date                    # Current date/time

## Processes & system
uname -a                # System info
whoami                  # Current user
id                      # User/group IDs
top                     # Live processes
htop                    # Nice top (if installed)
ps aux                  # All processes
pgrep nginx             # Find PIDs by name
kill -9 PID             # Kill process
pkill -f "pattern"      # Kill by pattern
killall nginx           # Kill by name
nice -n 10 cmd          # Set process priority
renice -n 5 -p PID      # Change priority
nohup cmd &             # Run immune to hangups
dmesg -T                # Kernel messages (timestamped)
uptime                  # System load & uptime
free -h                 # Memory usage

## Services & logs (systemd)
systemctl status nginx  # Service status
systemctl start nginx   # Start service
systemctl stop nginx    # Stop service
systemctl enable nginx  # Enable at boot
journalctl -u nginx     # Logs for a unit
journalctl -f           # Follow system logs

## Networking
ip a                    # IP addresses
ip route                # Routing table
ping -c 4 example.com   # Connectivity test
curl -I https://site    # Fetch headers
wget https://file       # Download file
ssh user@host           # Remote shell
scp file user@host:/p   # Secure copy
rsync -av dir/ host:/p  # Sync directories
ss -tulpn               # Sockets (like netstat)
nc -lvkp 9000           # Netcat listen
traceroute example.com  # Route trace
dig example.com +short  # DNS lookup
nslookup example.com    # DNS (alternative)

## Permissions & users
chmod 755 file          # Permissions (rwxr-xr-x)
chown user:group file   # Change owner/group
umask                   # Default permission mask
adduser newuser         # Add user
passwd user             # Change password
sudo cmd                # Run as root

## Archives & compression
tar -cvf f.tar dir/     # Create tar
tar -xvf f.tar          # Extract tar
tar -czvf f.tgz dir/    # Create tar.gz
tar -xzvf f.tgz         # Extract tar.gz
gzip file               # Compress
gunzip file.gz          # Decompress
zip -r f.zip dir/       # Zip directory
unzip f.zip             # Unzip

## Package managers
apt update              # Debian/Ubuntu: update index
apt upgrade             # Upgrade packages
apt install pkg         # Install
apt remove pkg          # Remove
dnf install pkg         # Fedora/RHEL
yum install pkg         # Older RHEL/CentOS
pacman -S pkg           # Arch
snap install pkg        # Snap packages

## Editors & shells
nano file.txt           # Simple editor
vim file.txt            # Powerful editor
code .                  # VS Code (if installed)
bash                    # Bash shell
zsh                     # Zsh shell

## Scheduling & jobs
crontab -e              # Edit user cron
crontab -l              # List cron jobs
at 14:30                # Schedule one-off job
bg                      # Send job to background
fg                      # Bring job to foreground
jobs                    # List shell jobs

## Pipelines & tools
xargs -n1 echo          # Build commands from input
tee out.txt             # Split output to file and stdout
yes | cmd               # Auto-confirm
printf "%s\n" {1..5} | parallel echo {}  # GNU parallel

## Git & Docker (common)
git clone URL           # Clone repo
git status              # Repo status
git add .               # Stage changes
git commit -m "msg"     # Commit
git push                # Push changes
docker ps               # Running containers
docker logs -f NAME     # Follow container logs
docker exec -it NAME sh # Shell into container

---

# Memory Allocation in Kernel Space

## `kmalloc` - Kernel Memory Allocation

```c
#include <linux/slab.h>

void *ptr = kmalloc(size, GFP_KERNEL);
if (!ptr) {
    pr_err("Memory allocation failed\n");
    return -ENOMEM;
}

kfree(ptr);
```

- Allocates contiguous physical memory
- `GFP_KERNEL` → can sleep, used in normal context
- `GFP_ATOMIC` → cannot sleep, used in interrupt/atomic context
- Returns NULL on failure

## `kzalloc` - Zero-Initialized Allocation

```c
void *ptr = kzalloc(size, GFP_KERNEL);
```

- Same as `kmalloc` but initializes memory to zero
- Equivalent to `kmalloc` + `memset(..., 0, ...)`

## `vmalloc` - Virtual Memory Allocation

```c
void *ptr = vmalloc(size);
kfree(ptr);  // Use kfree, not vfree in modern kernels
```

- Allocates virtual memory (not necessarily contiguous physical)
- Slower than kmalloc but handles large allocations
- Used when physical contiguity not required

## `devm_kmalloc` - Device-Managed Memory

```c
void *ptr = devm_kmalloc(&device->dev, size, GFP_KERNEL);
```

- Automatically freed when device is removed
- No need for manual kfree

---

# String Functions in Kernel Space

## Buffer/Memory Operations

```c
#include <linux/string.h>

memcpy(dest, src, n);               // Copy n bytes
memset(ptr, value, n);              // Fill with value
memmove(dest, src, n);              // Safe overlapping copy
memcmp(s1, s2, n);                  // Compare n bytes
```

## String Operations

```c
strlen(str);                        // String length
strnlen(str, maxlen);               // Safe string length
strcpy(dest, src);                  // Copy string (unsafe!)
strncpy(dest, src, n);              // Safe copy n chars
strcmp(s1, s2);                     // Compare strings
strncmp(s1, s2, n);                 // Compare n chars
sprintf(buf, "fmt", args);          // Format to buffer
snprintf(buf, size, "fmt", args);   // Safe format
```

---

# Spinlocks - Synchronization Primitive

## Basic Spinlock

```c
#include <linux/spinlock.h>

static spinlock_t my_lock;

// In init function:
spin_lock_init(&my_lock);

// In code:
spin_lock(&my_lock);
// Critical section
spin_unlock(&my_lock);

// Irqsafe (disable interrupts):
spin_lock_irqsave(&my_lock, flags);
// Critical section
spin_unlock_irqrestore(&my_lock, flags);
```

## Read-Write Spinlock

```c
static rwlock_t rw_lock = RW_LOCK_UNLOCKED;

read_lock(&rw_lock);
// Read section
read_unlock(&rw_lock);

write_lock(&rw_lock);
// Write section
write_unlock(&rw_lock);
```

---

# Semaphores - Blocking Synchronization

## Binary Semaphore

```c
#include <linux/semaphore.h>

static struct semaphore my_sem;
sema_init(&my_sem, 1);              // Binary semaphore

down(&my_sem);                       // Acquire (blocking)
down_interruptible(&my_sem);         // Can be interrupted by signal
// Critical section
up(&my_sem);                         // Release
```

## Counting Semaphore

```c
sema_init(&my_sem, N);              // N = initial count
```

---

# Wait Queues - For Blocking I/O

## Basic Wait Queue

```c
#include <linux/wait.h>

static DECLARE_WAIT_QUEUE_HEAD(my_wq);

// Waiter (reader/user process):
wait_event(my_wq, condition);
wait_event_interruptible(my_wq, condition);

// Waker (driver):
wake_up(&my_wq);                    // Wake one
wake_up_all(&my_wq);                // Wake all
```

## Manual Wait Queue Usage

```c
// Waiter:
add_wait_queue(&my_wq, &wait);
current->state = TASK_INTERRUPTIBLE;
schedule();
remove_wait_queue(&my_wq, &wait);

// Waker:
wake_up(&my_wq);
```

---

# Kernel Timers

## Timer Setup

```c
#include <linux/timer.h>

static struct timer_list my_timer;

void timer_callback(struct timer_list *t)
{
    pr_info("Timer expired\n");
    // Reschedule if needed:
    mod_timer(&my_timer, jiffies + HZ);  // 1 second
}

// In init:
timer_setup(&my_timer, timer_callback, 0);
my_timer.expires = jiffies + HZ;
add_timer(&my_timer);

// In exit/cleanup:
del_timer_sync(&my_timer);
```

---

# Interrupt Handlers

## Request and Free IRQ

```c
#include <linux/interrupt.h>

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    pr_info("IRQ %d handled\n", irq);
    return IRQ_HANDLED;
}

// In init:
ret = request_irq(irq_num, my_irq_handler, IRQF_SHARED, "my_device", &dev_data);
if (ret) {
    pr_err("Failed to request IRQ\n");
    return ret;
}

// In exit:
free_irq(irq_num, &dev_data);
```

## IRQ Flags

```c
IRQF_SHARED       // Multiple drivers share IRQ
IRQF_DISABLED     // Old flag (deprecated)
IRQF_TRIGGER_RISING   // Rising edge trigger
IRQF_TRIGGER_FALLING  // Falling edge trigger
```

---

# Tasklets - Deferred Interrupt Processing

## Tasklet Definition

```c
#include <linux/interrupt.h>

void tasklet_func(struct tasklet_struct *t)
{
    pr_info("Tasklet running\n");
}

static DECLARE_TASKLET(my_tasklet, tasklet_func);

// Or dynamic:
struct tasklet_struct my_tasklet;
tasklet_init(&my_tasklet, tasklet_func, 0);

// Schedule from IRQ:
tasklet_schedule(&my_tasklet);

// Cleanup:
tasklet_kill(&my_tasklet);
```

---

# Work Queues - Deferred Work with Sleep

## Work Queue Definition

```c
#include <linux/workqueue.h>

static struct workqueue_struct *my_wq;

void work_func(struct work_struct *work)
{
    pr_info("Work function running\n");
}

static DECLARE_WORK(my_work, work_func);

// Or dynamic:
struct work_struct my_work;
INIT_WORK(&my_work, work_func);

// Create queue:
my_wq = create_workqueue("my_queue");

// Schedule work:
queue_work(my_wq, &my_work);

// Cleanup:
flush_workqueue(my_wq);
destroy_workqueue(my_wq);
```

---

# Linked Lists - Kernel Data Structure

## List Operations

```c
#include <linux/list.h>

struct my_node {
    int data;
    struct list_head list;
};

// Define list head:
static LIST_HEAD(my_list);

// Create and add node:
struct my_node *node = kmalloc(sizeof(*node), GFP_KERNEL);
node->data = 42;
list_add(&node->list, &my_list);       // Add at head
list_add_tail(&node->list, &my_list);  // Add at tail

// Iterate:
struct my_node *entry;
list_for_each_entry(entry, &my_list, list) {
    pr_info("Data: %d\n", entry->data);
}

// Delete:
list_del(&node->list);
kfree(node);
```

---

# Container Of Macro

## Retrieving Parent Structure

```c
#include <linux/kernel.h>

struct my_node {
    int id;
    struct list_head list;
};

// Given pointer to list member, get parent struct:
struct list_head *ptr = &entry->list;
struct my_node *parent = container_of(ptr, struct my_node, list);

pr_info("ID: %d\n", parent->id);
```

---

# Sysfs Attributes

## Attribute Show/Store

```c
#include <linux/sysfs.h>

static ssize_t my_attr_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
    return snprintf(buf, PAGE_SIZE, "value: %d\n", my_value);
}

static ssize_t my_attr_store(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t count)
{
    sscanf(buf, "%d", &my_value);
    return count;
}

static DEVICE_ATTR(my_attr, 0644, my_attr_show, my_attr_store);

// In probe:
device_create_file(&dev->dev, &dev_attr_my_attr);

// In remove:
device_remove_file(&dev->dev, &dev_attr_my_attr);
```

---

# Platform Driver Framework

## Platform Driver Structure

```c
#include <linux/platform_device.h>

static int my_probe(struct platform_device *pdev)
{
    pr_info("Device probed\n");
    // Initialize device
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    pr_info("Device removed\n");
    // Cleanup
    return 0;
}

static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my_driver",
        .owner = THIS_MODULE,
    },
};

module_platform_driver(my_driver);
```

---

# Device Tree Bindings Example

## Basic DTS Syntax

```dts
/ {
    node_name {
        compatible = "vendor,device-name";
        reg = <0x1000 0x100>;          // Address and size
        interrupts = <10>;              // IRQ number
        status = "okay";                // or "disabled"

        property = "value";
        number = <42>;
        array = <1 2 3 4>;
    };
};
```

## Device Tree in Kernel Driver

```c
#include <linux/of.h>

static const struct of_device_id my_match_table[] = {
    { .compatible = "vendor,device-name" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_match_table);

static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my_driver",
        .of_match_table = my_match_table,
    },
};

// In probe, read DT properties:
struct device_node *node = pdev->dev.of_node;
u32 value;
of_property_read_u32(node, "property_name", &value);
```

---

# Common GFP Flags

| Flag | Description |
|------|-------------|
| `GFP_KERNEL` | Normal allocation, can sleep |
| `GFP_ATOMIC` | Allocation cannot sleep (IRQ/interrupt safe) |
| `GFP_NOWAIT` | No waiting, no reclaim |
| `GFP_NOFAIL` | Allocation will not fail |
| `GFP_DMA` | Allocate from DMA-safe zone |
| `GFP_ZERO` | Clear allocated memory (like kzalloc) |

---

# Useful Macros and Helpers

## Common Kernel Macros

```c
#include <linux/kernel.h>

BUG_ON(condition);              // Kernel panic if true
WARN_ON(condition);             // Warn if true
WARN_ONCE(condition, msg);      // Warn once
likely(x)                       // Branch prediction hint
unlikely(x)                     // Branch prediction hint
min(a, b)                       // Minimum value
max(a, b)                       // Maximum value
min_t(type, a, b)               // Typed minimum
max_t(type, a, b)               // Typed maximum
ALIGN(val, align)               // Align value
IS_ALIGNED(val, align)          // Check alignment
DIV_ROUND_UP(x, y)              // Round-up division
```

## Common Loop Macros

```c
for_each_cpu(cpu, mask) { }     // Iterate CPUs
for_each_node(node) { }         // Iterate NUMA nodes
list_for_each(pos, head) { }    // Iterate list
list_for_each_safe(pos, n, head) { }  // Safe delete during iteration
```

---

# /proc Filesystem Programming

## Creating /proc Entries

```c
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int my_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Value: %d\n", my_value);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_proc_show, NULL);
}

static const struct proc_ops my_proc_ops = {
    .proc_open    = my_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

// In init:
proc_create("my_proc", 0644, NULL, &my_proc_ops);

// In exit:
remove_proc_entry("my_proc", NULL);
```

## Creating Subdirectory in /proc

```c
// In init:
struct proc_dir_entry *proc_dir = proc_mkdir("mydriver", NULL);
proc_create("status", 0644, proc_dir, &my_proc_ops);

// In exit:
remove_proc_entry("status", proc_dir);
remove_proc_entry("mydriver", NULL);
```

## Seq File Interface for Large Data

```c
static void *my_seq_start(struct seq_file *m, loff_t *pos)
{
    return (*pos == 0) ? (void *)1 : NULL;
}

static void *my_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
    (*pos)++;
    return (*pos < MAX_ITEMS) ? (void *)(*pos + 1) : NULL;
}

static void my_seq_stop(struct seq_file *m, void *v)
{
    // Cleanup if needed
}

static int my_seq_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Item %lld\n", (loff_t)v);
    return 0;
}

static struct seq_operations my_seq_ops = {
    .start = my_seq_start,
    .next  = my_seq_next,
    .stop  = my_seq_stop,
    .show  = my_seq_show,
};

static int my_seq_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
}
```

---

# /sys Filesystem and Sysfs Programming

## Creating sysfs Attributes

```c
#include <linux/sysfs.h>

// Simple attribute
static ssize_t attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", my_value);
}

static ssize_t attr_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    sscanf(buf, "%d", &my_value);
    return count;
}

static struct kobj_attribute my_attr = __ATTR(my_attr, 0644, attr_show, attr_store);
```

## Attribute Groups

```c
static struct attribute *my_attrs[] = {
    &my_attr.attr,
    &other_attr.attr,
    NULL,
};

static struct attribute_group my_group = {
    .attrs = my_attrs,
};

// In init:
ret = sysfs_create_group(&device->kobj, &my_group);

// In exit:
sysfs_remove_group(&device->kobj, &my_group);
```

## Kobject Management

```c
#include <linux/kobject.h>

struct kobject *kobj = kobject_create_and_add("my_kobj", NULL);
if (!kobj)
    return -ENOMEM;

// Add attributes to kobj
sysfs_create_file(kobj, &my_attr.attr);

// Cleanup
sysfs_remove_file(kobj, &my_attr.attr);
kobject_put(kobj);
```

---

# ISR (Interrupt Service Routine) Programming

## Fast/Early ISR - Top Half

```c
#include <linux/interrupt.h>

static irqreturn_t my_isr_top_half(int irq, void *dev_id)
{
    struct my_device *dev = (struct my_device *)dev_id;

    // Check if interrupt is from our device
    if (!is_our_irq(dev))
        return IRQ_NONE;

    // Do minimal work - read status, mask interrupt
    dev->status = read_register(dev->base + STATUS_REG);
    disable_irq_nosync(irq);  // Or mask the interrupt

    // Schedule bottom half
    tasklet_schedule(&dev->tasklet);

    return IRQ_HANDLED;
}

// Register:
ret = request_irq(irq_num, my_isr_top_half, IRQF_SHARED, "my_dev", dev);
```

## Slow/Deferred ISR - Bottom Half (Tasklet)

```c
static void my_isr_bottom_half(struct tasklet_struct *t)
{
    struct my_device *dev = from_tasklet(dev, t, tasklet);

    // Do heavy work here
    process_data(dev);

    // Re-enable interrupt
    enable_irq(dev->irq);
}

// In init:
tasklet_init(&dev->tasklet, my_isr_bottom_half, 0);
```

## Return Values

```c
IRQ_NONE       // Interrupt not from this device
IRQ_HANDLED    // Interrupt handled successfully
IRQ_WAKE_THREAD // Wake kernel thread (threaded IRQ)
```

## Shared Interrupts Handling

```c
// Multiple devices on same IRQ line
static irqreturn_t my_shared_isr(int irq, void *dev_id)
{
    struct my_device *dev = (struct my_device *)dev_id;

    // Must check if interrupt is from our device
    if (!(readl(dev->base + INT_STATUS) & INT_BIT))
        return IRQ_NONE;  // Not our interrupt

    // Handle interrupt
    handle_interrupt(dev);

    return IRQ_HANDLED;
}

// Register with IRQF_SHARED flag:
request_irq(irq, my_shared_isr, IRQF_SHARED, "my_dev", dev);
```

## Threaded IRQ Handlers

```c
static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    // Top half - quick work
    return IRQ_WAKE_THREAD;
}

static irqreturn_t my_irq_thread_fn(int irq, void *dev_id)
{
    // Bottom half - can sleep
    return IRQ_HANDLED;
}

// Register:
request_threaded_irq(irq, my_irq_handler, my_irq_thread_fn,
                     IRQF_ONESHOT, "my_dev", dev);
```

## Context Restrictions in ISR

```c
// SAFE in ISR:
- readl(), writel()          // I/O operations
- spin_lock_irq()            // Disable interrupts
- atomic_inc()               // Atomic operations
- tasklet_schedule()         // Schedule tasklet
- wake_up()                  // Wake process (if with _interruptible)

// NOT SAFE in ISR:
- mutex_lock()               // Can sleep
- wait_event()               // Can sleep
- copy_from_user()           // Can sleep/fail
- kmalloc(GFP_KERNEL)        // Can sleep
- schedule()                 // Reschedule
```

---

# Synchronization Programming - Advanced Patterns

## Read-Copy-Update (RCU)

```c
#include <linux/rcu.h>

// Writer side:
static spinlock_t update_lock;

void update_data(void)
{
    spin_lock(&update_lock);

    // Copy old data to new
    struct my_data *new_data = kmalloc(sizeof(*new_data), GFP_KERNEL);
    memcpy(new_data, shared_data, sizeof(*new_data));

    // Modify new copy
    new_data->value = new_value;

    // Replace pointer
    rcu_assign_pointer(shared_data, new_data);

    spin_unlock(&update_lock);

    // Wait for readers to finish
    synchronize_rcu();

    // Free old data
    kfree(old_data);
}

// Reader side:
void read_data(void)
{
    rcu_read_lock();

    struct my_data *data = rcu_dereference(shared_data);
    int value = data->value;

    rcu_read_unlock();
}
```

## Atomic Operations

```c
#include <linux/atomic.h>

atomic_t counter = ATOMIC_INIT(0);

atomic_inc(&counter);              // Increment
atomic_dec(&counter);              // Decrement
atomic_add(n, &counter);           // Add n
atomic_sub(n, &counter);           // Subtract n
atomic_read(&counter);             // Read value
atomic_set(&counter, value);       // Set value
atomic_inc_return(&counter);       // Inc and return
atomic_dec_return(&counter);       // Dec and return
atomic_cmpxchg(&counter, old, new); // Compare and exchange
```

## Bit Operations - Atomic

```c
#include <linux/bitops.h>

unsigned long flags = 0;

set_bit(0, &flags);                // Set bit 0
clear_bit(0, &flags);              // Clear bit 0
test_bit(0, &flags);               // Test bit 0
test_and_set_bit(0, &flags);       // Test and set
test_and_clear_bit(0, &flags);     // Test and clear
change_bit(0, &flags);             // Toggle bit
```

## Completion - Wait for Event

```c
#include <linux/completion.h>

static DECLARE_COMPLETION(my_completion);

// Waiter:
wait_for_completion(&my_completion);

// Or with timeout:
unsigned long timeout = wait_for_completion_timeout(&my_completion, HZ);
if (timeout == 0)
    pr_err("Timeout waiting for completion\n");

// Signaler:
complete(&my_completion);
complete_all(&my_completion);  // Wake all waiters
```

## Memory Barriers

```c
#include <linux/barriersync.h>

// Full barrier - no operations cross
mb();                   // Memory barrier

// Read barrier - no reads cross
rmb();

// Write barrier - no writes cross
wmb();

// Compiler barrier - no code reordering
barrier();

// Acquire/Release semantics
smp_acquire__after_ctrl_dep();
smp_release__before_ctrl_dep();
```

## RW Semaphore

```c
#include <linux/rwsem.h>

static struct rw_semaphore rw_sem;
init_rwsem(&rw_sem);

// Reader:
down_read(&rw_sem);
// Read data
up_read(&rw_sem);

// Interruptible reader:
if (down_read_interruptible(&rw_sem))
    return -EINTR;
// Read data
up_read(&rw_sem);

// Writer:
down_write(&rw_sem);
// Write data
up_write(&rw_sem);
```

## Condition Variable Pattern

```c
// Setup:
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int condition = 0;

// Waiter:
wait_event_interruptible(wq, condition);

// Signaler:
condition = 1;
wake_up(&wq);

// Or manual:
add_wait_queue(&wq, &wait);
while (!condition) {
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
}
set_current_state(TASK_RUNNING);
remove_wait_queue(&wq, &wait);
```

## Per-CPU Variables

```c
#include <linux/percpu.h>

static DEFINE_PER_CPU(int, my_var);

// Get CPU's copy:
int *ptr = this_cpu_ptr(&my_var);
*ptr = 42;

// Or:
this_cpu_write(my_var, 42);
int val = this_cpu_read(my_var);

// From other CPU:
int val = per_cpu(my_var, cpu_id);
```

## Reference Counting

```c
#include <linux/refcount.h>

static refcount_t ref = REFCOUNT_INIT(1);

refcount_inc(&ref);           // Increment (panics on overflow)
refcount_dec(&ref);           // Decrement
refcount_dec_and_test(&ref);  // Dec and return true if zero
refcount_read(&ref);          // Read current value

// Usage pattern:
if (refcount_dec_and_test(&dev->ref)) {
    // Last reference released, cleanup
    kfree(dev);
}
```

## Try-Lock Pattern

```c
// Spinlock:
if (!spin_trylock(&lock)) {
    pr_err("Failed to acquire lock\n");
    return -EAGAIN;
}
// Critical section
spin_unlock(&lock);

// Mutex:
if (!mutex_trylock(&lock)) {
    pr_err("Failed to acquire lock\n");
    return -EAGAIN;
}
// Critical section
mutex_unlock(&lock);

// RW Semaphore:
if (!down_read_trylock(&rw_sem)) {
    return -EAGAIN;
}
// Read section
up_read(&rw_sem);
```

---

# Context and Sleeping Rules

| Context | Can Sleep? | Examples |
|---------|-----------|----------|
| `init` / `exit` | Yes | `module_init()`, `module_exit()` |
| `probe()` / `remove()` | Yes | Device driver initialization |
| User syscall context | Yes | `read()`, `write()`, `ioctl()` |
| Tasklet / BH | No | Bottom half handlers |
| IRQ handler (top half) | No | ISR - must complete quickly |
| Spinlock held | No | Inside `spin_lock()` section |
| Interrupt disabled | No | With interrupts disabled |
| Atomic context | No | `in_atomic()` returns true |
| Softirq context | No | Software interrupt handlers |

## Check Context at Runtime

```c
#include <linux/preempt.h>

if (in_interrupt())         // Inside any interrupt
    pr_info("In interrupt context\n");

if (in_softirq())           // Inside softirq/tasklet
    pr_info("In softirq context\n");

if (in_atomic())            // Cannot sleep
    pr_info("Atomic context\n");

if (preemptible())          // Can be preempted
    pr_info("Preemptible\n");
```

---

# Deadlock Avoidance

## Lock Ordering

```c
// Always acquire locks in the same order
// Order: lock_a -> lock_b -> lock_c

// Good:
spin_lock(&lock_a);
spin_lock(&lock_b);
spin_lock(&lock_c);
// Use resources
spin_unlock(&lock_c);
spin_unlock(&lock_b);
spin_unlock(&lock_a);

// Bad (different order):
spin_lock(&lock_c);  // Different order
spin_lock(&lock_a);  // Can cause deadlock!
```

## Nested Lock Safety

```c
// Spinlock is NOT reentrant - DEADLOCK if nested:
spin_lock(&lock);
// ... some code ...
spin_lock(&lock);  // DEADLOCK! Already held by this CPU
spin_unlock(&lock);
spin_unlock(&lock);

// Mutex IS reentrant:
mutex_lock(&lock);
// ... code ...
mutex_lock(&lock);  // OK on same thread
mutex_unlock(&lock);
mutex_unlock(&lock);

// Solution for spinlock: use different locks or irqsave
spin_lock_irqsave(&lock, flags);
// Cannot be interrupted, safe to reacquire later
```

## Priority Inversion Prevention

```c
// Use priority-inheriting locks when mixing priorities
#include <linux/rtmutex.h>

static struct rt_mutex prio_lock;
rt_mutex_init(&prio_lock);

rt_mutex_lock(&prio_lock);
// Critical section
rt_mutex_unlock(&prio_lock);
```

---

# GPIO Operations - New gpiod API

## GPIO Descriptor-Based API (Recommended)

```c
#include <linux/gpio/consumer.h>

// Request GPIO by name (from device tree)
struct gpio_desc *gpio = devm_gpiod_get(&pdev->dev, "my_gpio", GPIOD_OUT_LOW);
if (IS_ERR(gpio)) {
    pr_err("Failed to get GPIO\n");
    return PTR_ERR(gpio);
}

// Set GPIO value
gpiod_set_value(gpio, 1);          // Set HIGH
gpiod_set_value(gpio, 0);          // Set LOW

// Read GPIO value
int value = gpiod_get_value(gpio);

// Set direction
gpiod_direction_output(gpio, 1);   // Output mode, initial HIGH
gpiod_direction_input(gpio);       // Input mode

// Async/delayed set
gpiod_set_value_cansleep(gpio, 1); // For GPIO that needs sleep
```

## Old GPIO API (Legacy - Avoid)

```c
#include <linux/gpio.h>

int gpio_num = 42;
gpio_request(gpio_num, "my_gpio");
gpio_direction_output(gpio_num, 1);
gpio_set_value(gpio_num, 1);
gpio_get_value(gpio_num);
gpio_free(gpio_num);
```

---

# I/O Memory Access

## Memory-Mapped I/O Operations

```c
#include <linux/io.h>

// Map physical address to virtual address
void __iomem *base = ioremap(phys_address, size);
if (!base) {
    pr_err("ioremap failed\n");
    return -ENOMEM;
}

// Read/Write operations
u8 val8 = ioread8(base + offset);           // Read 8-bit
u16 val16 = ioread16(base + offset);        // Read 16-bit
u32 val32 = ioread32(base + offset);        // Read 32-bit

iowrite8(0xAB, base + offset);              // Write 8-bit
iowrite16(0xABCD, base + offset);           // Write 16-bit
iowrite32(0xABCDEF00, base + offset);       // Write 32-bit

// Read/Write multiple values
ioread32_rep(base, buf, count);             // Read count 32-bit values
iowrite32_rep(base, buf, count);            // Write count 32-bit values

// Unmap when done
iounmap(base);
```

## Device Resource Management

```c
#include <linux/ioport.h>

// Request memory region
struct resource *res = request_mem_region(start, size, "my_device");
if (!res) {
    pr_err("Memory region already in use\n");
    return -EBUSY;
}

// Map and use
void __iomem *base = ioremap(start, size);

// Cleanup
iounmap(base);
release_mem_region(start, size);

// Or use devm (automatic cleanup):
struct resource *res = devm_request_mem_region(&pdev->dev, start, size, "my_device");
void __iomem *base = devm_ioremap(&pdev->dev, start, size);
```

---

# Debugfs - Debug Filesystem

## Creating debugfs Entries

```c
#include <linux/debugfs.h>

// Create directory
struct dentry *debug_dir = debugfs_create_dir("mydriver", NULL);

// Create files
struct dentry *debug_file = debugfs_create_file("status", 0644,
                                                debug_dir, NULL,
                                                &debug_fops);

// Simple read/write
static ssize_t debug_read(struct file *file, char __user *buf,
                          size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(buf, count, ppos, "Debug info\n", 11);
}

static const struct file_operations debug_fops = {
    .owner = THIS_MODULE,
    .read  = debug_read,
};

// Helper functions
debugfs_create_u32("value", 0644, debug_dir, &my_value);  // 32-bit integer
debugfs_create_x32("flags", 0644, debug_dir, &my_flags);  // Hex 32-bit
debugfs_create_bool("enabled", 0644, debug_dir, &enabled); // Boolean

// Cleanup
debugfs_remove_recursive(debug_dir);
```

---

# DMA (Direct Memory Access)

## DMA Memory Allocation

```c
#include <linux/dma-mapping.h>

// Allocate DMA-safe memory
dma_addr_t dma_addr;
void *dma_buf = dma_alloc_coherent(&pdev->dev, size, &dma_addr, GFP_KERNEL);
if (!dma_buf) {
    pr_err("DMA allocation failed\n");
    return -ENOMEM;
}

// Use dma_buf and dma_addr for DMA operations
// dma_buf → virtual address for CPU
// dma_addr → physical address for DMA device

// Map user buffer for DMA
dma_addr_t dma_addr = dma_map_single(&pdev->dev, user_buf, size, DMA_TO_DEVICE);
if (dma_mapping_error(&pdev->dev, dma_addr)) {
    pr_err("DMA mapping failed\n");
    return -ENOMEM;
}

// Unmap after DMA completes
dma_unmap_single(&pdev->dev, dma_addr, size, DMA_TO_DEVICE);

// Cleanup allocated memory
dma_free_coherent(&pdev->dev, size, dma_buf, dma_addr);
```

## DMA Directions

```c
DMA_TO_DEVICE       // Data written to device
DMA_FROM_DEVICE     // Data read from device
DMA_BIDIRECTIONAL   // Both directions
```

---

# Clock Management

## Clock Control API

```c
#include <linux/clk.h>

// Get clock reference
struct clk *clk = devm_clk_get(&pdev->dev, "my_clock");
if (IS_ERR(clk)) {
    pr_err("Clock not found\n");
    return PTR_ERR(clk);
}

// Enable clock
clk_prepare_enable(clk);  // Prepare and enable (safe, can sleep)

// Or separately (for atomic contexts):
clk_prepare(clk);         // Can sleep - do in probe
clk_enable(clk);          // Cannot sleep - fast

// Get clock rate
unsigned long rate = clk_get_rate(clk);

// Set clock rate
clk_set_rate(clk, new_rate);

// Get/set parent clock
struct clk *parent = clk_get_parent(clk);
clk_set_parent(clk, parent);

// Disable clock
clk_disable_unprepare(clk); // Disable and unprepare

// Or separately:
clk_disable(clk);
clk_unprepare(clk);
```

---

# Power Management - Suspend/Resume

## Power Management Callbacks

```c
#include <linux/pm.h>

static int my_suspend(struct device *dev)
{
    pr_info("Device suspending\n");
    // Save state, disable interrupts
    return 0;
}

static int my_resume(struct device *dev)
{
    pr_info("Device resuming\n");
    // Restore state, enable interrupts
    return 0;
}

static const struct dev_pm_ops my_pm_ops = {
    .suspend = my_suspend,
    .resume  = my_resume,
};

// In platform driver:
static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my_driver",
        .pm   = &my_pm_ops,
    },
};
```

---

# Export Symbols - Module Symbol Visibility

## Exporting Functions

```c
// Make symbol available to other modules
EXPORT_SYMBOL(my_function);
EXPORT_SYMBOL_GPL(my_gpl_function);

// Example:
int my_exported_func(int value)
{
    return value * 2;
}
EXPORT_SYMBOL(my_exported_func);
```

## Importing Exported Symbols

```c
// In another module:
extern int my_exported_func(int value);

// Use it:
int result = my_exported_func(42);
```

---

# Miscellaneous Device Driver

## Misc Device Registration

```c
#include <linux/miscdevice.h>

static const struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .read  = misc_read,
    .write = misc_write,
    .open  = misc_open,
    .release = misc_release,
};

static struct miscdevice my_misc = {
    .minor = MISC_DYNAMIC_MINOR,  // Automatic minor number
    .name  = "my_device",         // Device name (/dev/my_device)
    .fops  = &misc_fops,
};

// In init:
ret = misc_register(&my_misc);
if (ret) {
    pr_err("Failed to register misc device\n");
    return ret;
}

// In exit:
misc_deregister(&my_misc);
```

---

# I2C Driver Framework

## I2C Driver Structure

```c
#include <linux/i2c.h>

static int my_i2c_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    pr_info("I2C device probed\n");
    // Initialize device
    return 0;
}

static int my_i2c_remove(struct i2c_client *client)
{
    pr_info("I2C device removed\n");
    // Cleanup
    return 0;
}

static const struct i2c_device_id my_i2c_ids[] = {
    { "my_device", 0 },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, my_i2c_ids);

static const struct of_device_id my_i2c_of_match[] = {
    { .compatible = "vendor,my-device" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_i2c_of_match);

static struct i2c_driver my_i2c_driver = {
    .probe    = my_i2c_probe,
    .remove   = my_i2c_remove,
    .id_table = my_i2c_ids,
    .driver   = {
        .name = "my_i2c_driver",
        .of_match_table = my_i2c_of_match,
    },
};

module_i2c_driver(my_i2c_driver);
```

## I2C Read/Write Operations

```c
// Write data to I2C device
static int write_to_i2c(struct i2c_client *client, u8 reg, u8 *data, int len)
{
    struct i2c_msg msg;
    u8 buf[256];

    buf[0] = reg;
    memcpy(&buf[1], data, len);

    msg.addr = client->addr;
    msg.flags = 0;              // Write
    msg.len = len + 1;
    msg.buf = buf;

    return i2c_transfer(client->adapter, &msg, 1);
}

// Read data from I2C device
static int read_from_i2c(struct i2c_client *client, u8 reg, u8 *data, int len)
{
    struct i2c_msg msgs[2];

    msgs[0].addr = client->addr;
    msgs[0].flags = 0;             // Write
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = client->addr;
    msgs[1].flags = I2C_M_RD;      // Read
    msgs[1].len = len;
    msgs[1].buf = data;

    return i2c_transfer(client->adapter, msgs, 2);
}
```

---

# SPI Driver Framework

## SPI Driver Structure

```c
#include <linux/spi/spi.h>

static int my_spi_probe(struct spi_device *spi)
{
    pr_info("SPI device probed\n");
    spi->bits_per_word = 8;
    spi->mode = SPI_MODE_0;
    spi_setup(spi);
    return 0;
}

static int my_spi_remove(struct spi_device *spi)
{
    pr_info("SPI device removed\n");
    return 0;
}

static const struct of_device_id my_spi_of_match[] = {
    { .compatible = "vendor,my-spi-device" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_spi_of_match);

static struct spi_driver my_spi_driver = {
    .probe  = my_spi_probe,
    .remove = my_spi_remove,
    .driver = {
        .name = "my_spi_driver",
        .of_match_table = my_spi_of_match,
    },
};

module_spi_driver(my_spi_driver);
```

## SPI Read/Write Operations

```c
// Single byte read/write
static int spi_write_byte(struct spi_device *spi, u8 byte)
{
    return spi_write_then_read(spi, &byte, 1, NULL, 0);
}

static int spi_read_byte(struct spi_device *spi, u8 *byte)
{
    return spi_write_then_read(spi, NULL, 0, byte, 1);
}

// Bulk transfer using spi_message
static int spi_bulk_transfer(struct spi_device *spi, u8 *tx, u8 *rx, int len)
{
    struct spi_transfer xfer = {
        .tx_buf = tx,
        .rx_buf = rx,
        .len = len,
    };
    struct spi_message msg;

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);

    return spi_sync(spi, &msg);
}
```

---

# Tracing and Debugging

## Trace Printk

```c
#include <linux/kernel.h>

// Like printk but visible in trace output (ftrace)
trace_printk("Debug: value = %d\n", my_value);

// View with: cat /sys/kernel/debug/tracing/trace
```

## Kgdb - Kernel Debugger (Basics)

```
Boot kernel with: kgdboc=ttyS0,115200

In gdb:
(gdb) target remote /dev/ttyS0
(gdb) break my_function
(gdb) continue
(gdb) print my_var
(gdb) step
```

## Ftrace - Function Tracing

```bash
# Enable tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Trace specific function
echo my_function > /sys/kernel/debug/tracing/set_ftrace_filter

# View trace
cat /sys/kernel/debug/tracing/trace

# Disable tracing
echo 0 > /sys/kernel/debug/tracing/tracing_on
```

---

# Netlink Sockets - Kernel-User Communication

## Basic Netlink Setup

```c
#include <linux/netlink.h>
#include <net/netlink.h>

// Define custom netlink family
#define MY_NETLINK_FAMILY NETLINK_USERSOCK  // or NETLINK_GENERIC

// Receive messages from userspace
static void my_netlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;

    if (nlh->nlmsg_len < sizeof(*nlh)) {
        return;
    }

    pr_info("Received netlink message: %s\n", (char *)NLMSG_DATA(nlh));
}

static struct netlink_kernel_cfg cfg = {
    .input = my_netlink_recv_msg,
};

// In init:
struct sock *nl_sock = netlink_kernel_create(&init_net, MY_NETLINK_FAMILY, &cfg);
if (!nl_sock) {
    pr_err("Failed to create netlink socket\n");
    return -ENOMEM;
}

// In exit:
netlink_kernel_release(nl_sock);
```

## Send Message to Userspace

```c
static void send_netlink_msg(struct sock *nl_sock, const char *msg)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int msg_size = strlen(msg);
    int res;

    skb = nlmsg_new(NLMSG_ALIGN(msg_size + 1), GFP_KERNEL);
    if (!skb) {
        pr_err("Failed to allocate netlink message\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, msg_size + 1, 0);
    if (!nlh) {
        kfree_skb(skb);
        return;
    }

    memcpy(NLMSG_DATA(nlh), msg, msg_size + 1);

    res = netlink_broadcast(nl_sock, skb, 0, 0, GFP_KERNEL);
    if (res < 0) {
        pr_err("Failed to broadcast netlink message: %d\n", res);
    }
}
```

---

# Module Versioning and Dependencies

## Module Dependencies

```c
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author Name");
MODULE_DESCRIPTION("Driver description");
MODULE_VERSION("1.0");

// Declare soft dependency on another module
MODULE_SOFTDEP("pre: other_module");   // Load before
MODULE_SOFTDEP("post: other_module");  // Load after
```

## Module Aliases

```c
// Allow loading by alternative name
MODULE_ALIAS("device:my_device_name");
MODULE_ALIAS("platform:my_platform_device");

// Check module info:
modinfo my_module.ko
```

---

# Quick Reference - Common Patterns

## Error Handling

```c
// Check pointer allocation
if (!ptr) {
    pr_err("Allocation failed\n");
    return -ENOMEM;
}

// Check error pointer (for functions returning PTR_ERR)
if (IS_ERR(ptr)) {
    int err = PTR_ERR(ptr);
    pr_err("Function failed with error: %d\n", err);
    return err;
}

// Check ioctl error
if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
    return -EFAULT;

// Check return value
ret = some_function();
if (ret < 0) {
    pr_err("Function failed: %d\n", ret);
    return ret;
}
```

## Resource Cleanup Pattern

```c
// Devm (device-managed) - automatic cleanup
ptr = devm_kmalloc(&pdev->dev, size, GFP_KERNEL);
clk = devm_clk_get(&pdev->dev, "clock");
gpio = devm_gpiod_get(&pdev->dev, "gpio", flags);

// Manual cleanup - use goto error handling
ret = request_irq(irq, handler, flags, "device", dev);
if (ret)
    goto err_irq;

ret = register_device(dev);
if (ret)
    goto err_register;

return 0;

err_register:
    free_irq(irq, dev);
err_irq:
    return ret;
```

---

# Kernel Threads - kthread API

## Kernel Thread Structure

```c
#include <linux/kthread.h>

struct task_struct *th;

static int thread_func(void *data)
{
    const char *name = (const char *)data;

    while (!kthread_should_stop()) {
        // Do work
        msleep(1000);
    }

    return 0;
}

// Create and start
th = kthread_create(thread_func, "thread_name", "format_%s");
wake_up_process(th);

// Or create and wake in one call
th = kthread_run(thread_func, "thread_name", "format_%s");

// Stop thread
kthread_stop(th);
```

## Key Functions

| Function | Purpose |
|----------|---------|
| `kthread_create()` | Create thread (not started) |
| `kthread_run()` | Create and start thread |
| `wake_up_process()` | Wake sleeping thread |
| `kthread_should_stop()` | Check if stop requested |
| `kthread_stop()` | Stop and wait for thread |
| `msleep()`, `usleep_range()` | Sleep in thread |
| `smp_processor_id()` | Get current CPU ID |

---

# DEVICE_ATTR Macros

```c
// Read-only attribute
static DEVICE_ATTR_RO(name);

// Write-only attribute
static DEVICE_ATTR_WO(name);

// Read-Write attribute
static DEVICE_ATTR_RW(name);

// Attribute with custom permissions
static DEVICE_ATTR(name, 0644, show_func, store_func);
```

---

# Attribute Group Helpers

```c
static struct attribute *my_attrs[] = {
    &dev_attr_name.attr,
    NULL,
};

static struct attribute_group my_group = {
    .attrs = my_attrs,
};

// Register in probe
sysfs_create_group(&pdev->dev.kobj, &my_group);

// Unregister in remove
sysfs_remove_group(&pdev->dev.kobj, &my_group);
```

---

# DECLARE and INIT Macros

| Macro | Purpose |
|-------|---------|
| `DECLARE_TASKLET(name, func, data)` | Declare tasklet |
| `DECLARE_WORK(name, func)` | Declare work queue item |
| `DECLARE_WAIT_QUEUE_HEAD(name)` | Declare wait queue |
| `LIST_HEAD(name)` | Declare linked list head |
| `DEFINE_MUTEX(name)` | Declare and initialize mutex |
| `DEFINE_SPINLOCK(name)` | Declare and initialize spinlock |
| `atomic_t var = ATOMIC_INIT(0)` | Declare atomic counter |
