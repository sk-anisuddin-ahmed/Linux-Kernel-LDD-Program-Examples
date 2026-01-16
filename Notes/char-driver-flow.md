# Linux Character Device Driver Flow

This document explains what a character device driver is, how it loads, and what changes it makes in Linux directories like `/dev`, `/sys`, and `/proc` — in a clear, beginner-friendly but technical way.

---

## 1. What is a Character Device Driver?

A character device driver is a Linux kernel driver that:
- Transfers data byte-by-byte
- Is accessed using file operations
- Appears to user space like a file

**Typical examples:**
- UART / Serial drivers
- GPIO drivers
- I2C / SPI devices
- Simple custom hardware drivers

---

## 2. High-Level Flow (Big Picture)

```
User loads driver (insmod)
        ↓
Driver registers device in kernel
        ↓
sysfs exposes device in /sys
        ↓
udev creates device node in /dev
        ↓
User reads/writes via /dev
```

---

## 3. Step-by-Step Character Driver Flow

### STEP 0 — Before driver is loaded

- `/dev` → no device node
- `/sys` → no device entry
- `/proc` → no driver entry

Nothing exists yet.

---

### STEP 1 — Load the driver

```bash
sudo insmod my_char.ko
```

**Kernel actions:**
- Loads `.ko` into kernel RAM
- Resolves symbols & relocations
- Calls `module_init()`

**Filesystem effect:** No visible change yet.

---

### STEP 2 — Allocate device number

```c
alloc_chrdev_region(&dev, 0, 1, "mychar");
```

- Assigns major:minor number (e.g., `240:0`)
- Stored internally in kernel tables

**Filesystem effect:** No visible files created.

---

### STEP 3 — Register file operations

```c
cdev_init(&my_cdev, &fops);
cdev_add(&my_cdev, dev, 1);
```

- Links major/minor to `file_operations`
- Kernel now knows which functions to call on read/write

**Filesystem effect:** Still nothing visible.

---

### STEP 4 — Register with device model (CRITICAL STEP)

```c
class_create(THIS_MODULE, "myclass");
device_create(myclass, NULL, dev, NULL, "mychar0");
```

**Kernel creates:**
- `struct class`
- `struct device`

**Filesystem effect:**
```
/sys/class/myclass/mychar0/
```

✅ sysfs entry is created.

---

### STEP 5 — udev creates `/dev` node automatically

- udev reads: `/sys/class/myclass/mychar0/dev`
- Example content: `240:0`
- udev creates: `/dev/mychar0`

✅ Device node appears.

---

### STEP 6 — User accesses the device

```bash
echo "hello" > /dev/mychar0
cat /dev/mychar0
```

**Kernel call path:**
- `write()` → VFS → `driver_write()`
- `read()` → VFS → `driver_read()`

**Filesystem effect:** No new files, but `/dev/mychar0` is actively used.

---

### STEP 7 — Optional: Add sysfs attributes

```c
DEVICE_ATTR(mode, 0644, show, store);
```

**Filesystem effect:**
```
/sys/class/myclass/mychar0/mode
```

Used for small configuration or state.

---

### STEP 8 — Optional: Add proc entry

```c
proc_create("mychar", 0, NULL, &proc_ops);
```

**Filesystem effect:**
```
/proc/mychar
```

Used for debug, counters, runtime information.

---

### STEP 9 — Remove the driver

```bash
sudo rmmod my_char
```

**Kernel cleanup:**
- Removes device & class
- Frees kernel memory

**Filesystem effect:**
- `/dev/mychar0` → removed
- `/sys/class/myclass` → removed
- `/proc/mychar` → removed

---

## 4. Role of Each Directory (Very Important)

### `/dev` — Data Path

- Entry point for read/write/ioctl
- Used by applications
- Represents access to driver
- Example: `/dev/mychar0`

### `/sys` — Control & Discovery (sysfs)

- Virtual filesystem exposing kernel objects
- Shows devices, drivers, attributes
- Used for configuration and state
- Example: `/sys/class/myclass/mychar0/`

### `/proc` — Runtime & Debug Information

- Shows live kernel state
- Process info, memory, modules
- Optional for drivers
- Example: `/proc/mychar`

---

## 5. sysfs vs /dev vs /proc (Comparison Table)

| Directory | Purpose        | Used For          |
|-----------|----------------|-------------------|
| `/dev`    | Data I/O       | read/write bytes  |
| `/sys`    | Control & info | device attributes |
| `/proc`   | Runtime state  | debug & stats     |

---

## 6. Key Rules to Remember

1. Devices live in kernel memory, not in filesystem.
2. `/sys` describes devices.
3. `/dev` uses devices.
4. `/proc` shows kernel runtime state.
5. sysfs comes before `/dev`.

---

## 7. Final One-Line Summary

A character device driver creates kernel objects; sysfs exposes them, udev creates `/dev` for data access, and `/proc` is optionally used for runtime/debug information.
