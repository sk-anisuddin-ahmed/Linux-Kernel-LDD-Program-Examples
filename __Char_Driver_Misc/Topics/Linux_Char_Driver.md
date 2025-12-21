# 🧠 Linux Kernel Modules & Character Device Drivers

### PART 1 — Linux Kernel & Module Fundamentals

1. Linux Kernel Basics
   - What is the Linux Kernel
   - Kernel space vs User space
   - Monolithic kernel concept
   - What runs in kernel space
   - Why kernel programming is dangerous

2. Device Drivers
   - What is a Device Driver
   - Why device drivers are needed
   - Where device drivers run
   - Device driver responsibilities
   - Examples: GPIO, UART, I2C, SPI, Storage

3. Kernel Modules
   - What is a Kernel Module
   - Why kernel modules exist
   - Loadable Kernel Modules (LKM)
   - Advantages over static kernel code
   - Another name for kernel module (LKM)

4. Device Drivers vs Kernel Modules
   - Driver as a concept
   - Module as a delivery mechanism
   - Can a module exist without being a driver?
   - Can a driver exist without being a module?

5. Types of Kernel Modules
   - Character driver modules
   - Block driver modules
   - Network driver modules
   - Pseudo modules
   - Built-in vs Loadable modules

### PART 2 — Kernel Module Development Basics

6. Kernel Module Lifecycle
   - module_init()
   - module_exit()
   - What happens if init fails
   - What happens if exit is missing
   - What happens if only init or only exit exists

7. Hello World Kernel Module
   - Minimal kernel module structure
   - Headers required
   - printk usage
   - MODULE_LICENSE, MODULE_AUTHOR
   - Loading and unloading basics

8. printk vs printf
   - Why printf doesn't work in kernel
   - printk internals
   - Kernel ring buffer
   - printk formatting rules

9. Kernel Module Build System
   - Simplified Makefile
   - Kbuild basics
   - obj-m vs obj-y
   - Multiple C files in one module
   - Multiple modules from one Makefile

10. Kernel Module Loading
    - insmod internal working
    - modprobe internal working
    - Difference between insmod and modprobe
    - Dependency handling
    - modules.dep and depmod
    - strace on insmod

11. From .c to .ko
    - Compilation stages
    - Object files
    - Linking
    - Module.symvers
    - modules.order
    - Version magic

### PART 3 — Kernel Logging & Debugging

12. dmesg Deep Dive
    - Kernel ring buffer
    - dmesg output format
    - dmesg -w (follow)
    - Log persistence

13. printk Internals
    - printk log levels
    - Default log level
    - printk macros (pr_info, pr_err, etc.)
    - Rate limiting
    - printk_once
    - Avoiding newline behavior
    - Printing hex dumps
    - Dynamic debug
    - Why floating point is disabled in kernel

14. Kernel Failures
    - Kernel panic
    - Oops
    - BUG()
    - Difference between panic, oops, bug
    - Can module be removed after oops?
    - Dumping kernel stack

### PART 4 — Module Metadata & Licensing

15. Module Metadata
    - MODULE_LICENSE
    - MODULE_AUTHOR
    - MODULE_DESCRIPTION
    - MODULE_INFO
    - Objdump on kernel modules

16. Kernel Licensing
    - GPL vs non-GPL modules
    - Tainted kernel
    - Checking tainted state
    - Invalid license behavior
    - GPL-only symbols

### PART 5 — Kernel Symbols & Internals

17. Symbols & Symbol Tables
    - What is a kernel symbol
    - Exporting symbols
    - EXPORT_SYMBOL vs EXPORT_SYMBOL_GPL
    - Module stacking
    - System.map
    - /proc/kallsyms

### PART 6 — Kernel Version Handling

18. Kernel Version Macros
    - LINUX_VERSION_CODE
    - KERNEL_VERSION
    - UTS_RELEASE
    - Supporting multiple kernel versions
    - Preprocessor output analysis

### PART 7 — Kernel Memory & Sections

19. Kernel Memory Sections
    - __init
    - __exit
    - __initdata
    - __exitdata
    - What happens after module load
    - Built-in module behavior

20. Module Management
    - Built-in modules
    - Automatic module loading
    - Blacklisting modules
    - modprobe configuration
    - systool utility

### PART 8 — Process & Thread Context (Kernel Side)

21. Process Representation
    - task_struct
    - Process states
    - current macro
    - Printing process info
    - Memory map of process

22. Kernel Threads
    - What is a kernel thread
    - kthread_create
    - kthread_run
    - kthread_stop
    - kthread_should_stop
    - Naming threads
    - CPU ID printing
    - Race conditions

### PART 9 — Character Device Drivers (Core)

23. Character vs Block Devices
    - Differences
    - Examples
    - Where they appear in filesystem

24. Device Numbers
    - Major number
    - Minor number
    - dev_t type
    - MAJOR, MINOR, MKDEV
    - /proc/devices
    - Static allocation
    - Dynamic allocation
    - Maximum limits

25. Device Node Creation
    - Manual mknod
    - Automatic creation
    - class_create
    - device_create
    - Multiple device nodes

### PART 10 — File Operations & VFS

26. file_operations
    - open
    - release
    - read
    - write
    - lseek
    - unlocked_ioctl

27. struct file
    - What it represents
    - Per-open instance data
    - f_pos
    - private_data

28. struct inode
    - What it represents
    - Relationship with file
    - inode vs file comparison

29. Fork & Multiple Processes
    - open/release behavior with fork
    - Multiple process access
    - Exclusive access logic

### PART 11 — User Space ↔ Kernel Space Communication

30. User Space Access Rules
    - Why user pointers are invalid in kernel
    - What happens if NULL is passed
    - access_ok macro

31. Data Transfer APIs
    - copy_from_user
    - copy_to_user
    - get_user
    - put_user
    - strnlen_user
    - Passing structures
    - Heap vs stack pointers

### PART 12 — Read / Write Internals

32. Read & Write Logic
    - Offset handling
    - Partial reads/writes
    - Character-by-character issues
    - lseek support
    - Multiple devices handling

### PART 13 — ioctl Interface

33. ioctl Fundamentals
    - Why ioctl exists
    - BLKGETSIZE examples
    - unlocked_ioctl
    - Defining ioctl commands
    - _IO,_IOR, _IOW,_IOWR
    - Decoding ioctl commands

34. ioctl Safety
    - Size mismatch issues
    - Unsupported command handling
    - compat_ioctl
    - 32-bit vs 64-bit issues

### PART 14 — Access Control & Security

35. Access Restrictions
    - Single process access
    - Single user access
    - Reference counting
    - Capabilities framework

36. Capabilities
    - cap_dac_override
    - cap_sys_module
    - cap_sys_admin
    - When and why to use capabilities

### PART 15 — Signals & Misc Drivers

37. Signals from Kernel
    - Sending signal to process
    - Use cases
    - Safety considerations

38. Misc Drivers
    - What is misc driver
    - When to use misc framework
    - Advantages over manual char drivers
