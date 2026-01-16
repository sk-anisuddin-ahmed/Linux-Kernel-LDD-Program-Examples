# Linux `seq_file` Complete Guide

> A complete, practical guide to implementing `/proc` files using `seq_file` in Linux kernel.

---

## 1. Why `seq_file` Exists

When a user runs:

```bash
cat /proc/myfile
```

Internally, Linux does:

```c
read(fd, buf, 4096);
read(fd, buf, 4096);
read(fd, buf, 4096);
...
```

The kernel:

* Does **not** guarantee a single read call
* Does **not** guarantee a large buffer
* May call your `.read()` handler multiple times

So a driver must handle:

* Partial reads
* Offsets (`*ppos`)
* EOF
* Buffer limits
* Large outputs

This approach is **complex and error-prone**.

`seq_file` solves this problem by providing a **streaming text output engine**.

---

## 2. What `seq_file` Is

> `seq_file` is a kernel framework that:

* Manages a kernel buffer
* Handles offsets
* Handles chunked reads
* Handles large output
* Handles restartable iteration
* Handles `lseek()`
* Handles multiple readers
* Calls your code only to **generate data**

You only write:

```c
seq_printf(m, "..."); 
```

---

## 3. Two Ways to Use `seq_file`

There are **TWO practical usage patterns**:

| Method                  | Use Case                                         |
| ----------------------- | ------------------------------------------------ |
| `single_open()`         | Print ONE block of info (status, config, stats)  |
| `seq_open()` + iterator | Print LIST of objects (processes, devices, etc.) |

---

## 4. Method 1: `single_open()` (Most Common)

### When to Use

* Output is:

  * status
  * statistics
  * summary
  * configuration
* Data size is **reasonable**
* You want to print **everything at once**

### How It Works

Your show function is called once, and all output is generated in a single pass.

### Template Code

**Basic Example:**

```c
static int my_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Hello\n");
    seq_printf(m, "Value: %d\n", x);
    return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_show, NULL);
}

static const struct proc_ops my_fops = {
    .proc_open    = my_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
```

**Real-World Example (System Status):**

```c
static unsigned long system_uptime_ms = 0;
static int system_status = 1;

static int system_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Status Information\n");
    seq_printf(m, "==================\n");
    seq_printf(m, "System Status: %s\n", system_status ? "ONLINE" : "OFFLINE");
    seq_printf(m, "Uptime (ms): %lu\n", system_uptime_ms);
    seq_printf(m, "Kernel Version: %s\n", utsname()->release);
    seq_printf(m, "Current Time: %lld\n", ktime_get_ns());
    return 0;
}

static int system_open(struct inode *inode, struct file *file)
{
    return single_open(file, system_show, NULL);
}

static const struct proc_ops system_fops = {
    .proc_open    = system_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
```

**Example with Private Data:**

```c
struct device_info {
    char name[64];
    int temperature;
    int voltage;
};

static int device_show(struct seq_file *m, void *v)
{
    struct device_info *dev = (struct device_info *)m->private;
    
    seq_printf(m, "Device: %s\n", dev->name);
    seq_printf(m, "Temperature: %d°C\n", dev->temperature);
    seq_printf(m, "Voltage: %dV\n", dev->voltage);
    
    return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
    struct device_info *dev = (struct device_info *)PDE_DATA(inode);
    return single_open(file, device_show, dev);
}

static const struct proc_ops device_fops = {
    .proc_open    = device_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
```

### Call Flow

```
open()  -> single_open()
read()  -> seq_read() -> my_show()
read()  -> seq_read() returns 0
close() -> single_release()
```

### Advantages

* Extremely simple
* No buffer management
* No offset handling
* No copy_to_user()
* No EOF handling

### Limitations

* Builds **all output in memory**
* Bad for:

  * Huge lists
  * Many objects
  * Large dynamic data

---

## 5. Method 2: `seq_open()` with Iterator (For Lists)

### When to Use

* You want to print:

  * linked lists
  * arrays
  * tables
  * process list
  * device list
* Output can be:

  * large
  * unbounded
  * dynamic

### How It Works

Your iterator functions are called repeatedly to print a sequence of objects one at a time.

### Required Implementation

```c
struct seq_operations {
    void * (*start)(struct seq_file *, loff_t *);
    void * (*next)(struct seq_file *, void *, loff_t *);
    void   (*stop)(struct seq_file *, void *);
    int    (*show)(struct seq_file *, void *);
};
```

### Template Code

**Basic Iterator Pattern:**

```c
static void *my_start(struct seq_file *m, loff_t *pos)
{
    if (*pos == 0)
        return SEQ_START_TOKEN;
    return NULL;
}

static void *my_next(struct seq_file *m, void *v, loff_t *pos)
{
    return NULL;  // Only one item
}

static void my_stop(struct seq_file *m, void *v)
{
    // Cleanup if needed
}

static int my_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Item data\n");
    return 0;
}

static const struct seq_operations my_seq_ops = {
    .start = my_start,
    .next  = my_next,
    .stop  = my_stop,
    .show  = my_show,
};

static int my_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
}

static const struct proc_ops my_fops = {
    .proc_open    = my_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};
```

**Real-World Example (List of Devices):**

```c
struct device {
    struct list_head list;
    char name[64];
    int id;
};

static LIST_HEAD(device_list);
static DEFINE_SPINLOCK(device_lock);

static void *device_start(struct seq_file *m, loff_t *pos)
{
    loff_t *spos = kmalloc(sizeof(loff_t), GFP_KERNEL);
    if (!spos)
        return ERR_PTR(-ENOMEM);

    *spos = *pos;
    spin_lock(&device_lock);
    
    if (*spos == 0)
        return SEQ_START_TOKEN;
    
    return NULL;
}

static void *device_next(struct seq_file *m, void *v, loff_t *pos)
{
    struct device *dev = (struct device *)v;
    
    if (v == SEQ_START_TOKEN)
        dev = list_first_entry(&device_list, struct device, list);
    else if (list_is_last(&dev->list, &device_list))
        return NULL;
    else
        dev = list_next_entry(dev, list);
    
    (*pos)++;
    return dev;
}

static void device_stop(struct seq_file *m, void *v)
{
    spin_unlock(&device_lock);
}

static int device_show(struct seq_file *m, void *v)
{
    if (v == SEQ_START_TOKEN) {
        seq_printf(m, "Device List\n");
        seq_printf(m, "%4s | %s\n", "ID", "Name");
        seq_printf(m, "-----|---------------------\n");
        return 0;
    }
    
    struct device *dev = (struct device *)v;
    seq_printf(m, "%4d | %s\n", dev->id, dev->name);
    return 0;
}

static const struct seq_operations device_seq_ops = {
    .start = device_start,
    .next  = device_next,
    .stop  = device_stop,
    .show  = device_show,
};

static int device_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &device_seq_ops);
}

static const struct proc_ops device_fops = {
    .proc_open    = device_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};
```

**Example with Array Iteration:**

```c
#define MAX_ITEMS 100
static struct item_data items[MAX_ITEMS];
static int num_items = 0;

static void *array_start(struct seq_file *m, loff_t *pos)
{
    if (*pos >= num_items)
        return NULL;
    return (void *)((long)(*pos));
}

static void *array_next(struct seq_file *m, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= num_items)
        return NULL;
    return (void *)((long)(*pos));
}

static void array_stop(struct seq_file *m, void *v)
{
    // No cleanup needed for array
}

static int array_show(struct seq_file *m, void *v)
{
    int index = (long)v;
    struct item_data *item = &items[index];
    
    seq_printf(m, "[%d] %s: value=%d\n", 
               index, item->name, item->value);
    return 0;
}

static const struct seq_operations array_seq_ops = {
    .start = array_start,
    .next  = array_next,
    .stop  = array_stop,
    .show  = array_show,
};

static int array_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &array_seq_ops);
}

static const struct proc_ops array_fops = {
    .proc_open    = array_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};
```

### Call Flow

```
pos = 0
start(pos)
show(item0)
next()
show(item1)
next()
...
stop()
```

When the internal buffer fills:

* `seq_file` stops iteration
* Returns the buffered data to the user
* The next `read()` call resumes from the same position

### Advantages

* Streams data efficiently
* Minimal memory usage
* Safe for large lists
* Can handle millions of objects
* Works correctly with `head`, `less`, `tail`, and other tools

---

## 6. How `seq_read()` Works

The kernel framework handles the following steps:

```c
while (user buffer has space) {
    if (internal buffer is empty)
        call your show() or iterator to refill it

    copy bytes from internal buffer to user buffer
    update position offsets
}
```

You never need to call `copy_to_user()`—`seq_file` handles this automatically.

---

## 7. The `struct seq_file` Structure (Simplified)

```c
struct seq_file {
    char   *buf;    // internal buffer
    size_t  size;   // buffer size
    size_t  count;  // valid data
    loff_t  index;  // logical position
};
```

---

## 8. `seq_file` vs. Manual Implementation

| Feature       | Naive  | seq_file  |
| ------------- | ------ | --------- |
| Partial reads | Manual | Automatic |
| Offsets       | Manual | Automatic |
| Large output  | Hard   | Easy      |
| Lists         | Hard   | Natural   |
| Memory        | Risky  | Safe      |
| Bugs          | Many   | Few       |

---

## 9. When Not to Use `seq_file`

Almost never.

Only avoid if:

* Output is tiny
* Static
* Fixed
* Performance critical (rare)

---

## 10. Quick Decision Table

| Need            | Recommended Method  |
| --------------- | ------------------- |
| Status page     | `single_open()`     |
| Statistics      | `single_open()`     |
| Config dump     | `single_open()`     |
| List of objects | `seq_open()` + iterator |
| Unknown size    | `seq_open()` + iterator |

## 11. Summary

`seq_file` is a kernel framework that handles the complexity of streaming text output to `/proc` files. It manages buffering, offsets, and multiple reads so you can focus on generating data.

## 12. Registration in Module Initialization

### Method 1 Module Init (single_open)

```c
static int __init my_module_init(void)
{
    struct proc_dir_entry *entry;
    
    entry = proc_create("my_status", 0444, NULL, &my_fops);
    if (!entry) {
        printk(KERN_ERR "Failed to create /proc/my_status\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "Module loaded, /proc/my_status created\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    remove_proc_entry("my_status", NULL);
    printk(KERN_INFO "Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
```

### Method 2 Module Init (seq_open with iterator)

```c
static int __init device_module_init(void)
{
    struct proc_dir_entry *entry;
    struct device *dev1, *dev2;
    
    // Initialize device list
    dev1 = kmalloc(sizeof(struct device), GFP_KERNEL);
    if (!dev1)
        return -ENOMEM;
    
    strcpy(dev1->name, "Device_0");
    dev1->id = 0;
    list_add(&dev1->list, &device_list);
    
    dev2 = kmalloc(sizeof(struct device), GFP_KERNEL);
    if (!dev2) {
        kfree(dev1);
        return -ENOMEM;
    }
    
    strcpy(dev2->name, "Device_1");
    dev2->id = 1;
    list_add(&dev2->list, &device_list);
    
    // Create proc file
    entry = proc_create("devices", 0444, NULL, &device_fops);
    if (!entry) {
        printk(KERN_ERR "Failed to create /proc/devices\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "Device module loaded\n");
    return 0;
}

static void __exit device_module_exit(void)
{
    struct device *dev, *tmp;
    
    remove_proc_entry("devices", NULL);
    
    // Clean up device list
    list_for_each_entry_safe(dev, tmp, &device_list, list) {
        list_del(&dev->list);
        kfree(dev);
    }
    
    printk(KERN_INFO "Device module unloaded\n");
}

module_init(device_module_init);
module_exit(device_module_exit);
```

## 13. Complete Working Module Example

**Simple single_open Module:**

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("seq_file example module");

static unsigned int counter = 0;

static int counter_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Counter Value: %u\n", counter);
    seq_printf(m, "Total Reads: %u\n", counter++);
    seq_printf(m, "Status: RUNNING\n");
    return 0;
}

static int counter_open(struct inode *inode, struct file *file)
{
    return single_open(file, counter_show, NULL);
}

static const struct proc_ops counter_fops = {
    .proc_open    = counter_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init counter_module_init(void)
{
    struct proc_dir_entry *entry;
    
    entry = proc_create("counter", 0444, NULL, &counter_fops);
    if (!entry)
        return -ENOMEM;
    
    printk(KERN_INFO "Counter module loaded. Use: cat /proc/counter\n");
    return 0;
}

static void __exit counter_module_exit(void)
{
    remove_proc_entry("counter", NULL);
    printk(KERN_INFO "Counter module unloaded\n");
}

module_init(counter_module_init);
module_exit(counter_module_exit);
```

## 14. Usage Examples

### Reading Your proc File

```bash
# Basic read
cat /proc/counter

# Multiple reads
cat /proc/counter
cat /proc/counter

# With head/tail
head -n 5 /proc/counter
tail -n 2 /proc/counter

# Watch continuously
watch 'cat /proc/counter'

# In a script
while read line; do
    echo "Line: $line"
done < /proc/counter
```

## 15. Common Mistakes

**1. Returning non-zero from show (not EOF):**

```c
// WRONG
static int my_show(struct seq_file *m, void *v) {
    seq_printf(m, "data\n");
    return 1;  // Wrong! Should be 0
}

// CORRECT
static int my_show(struct seq_file *m, void *v) {
    seq_printf(m, "data\n");
    return 0;  // Always return 0 on success
}
```

**2. Forgetting to release locks in iterator:**

```c
// WRONG - Lock not released
static void my_stop(struct seq_file *m, void *v) {
    // Forgot to unlock!
}

// CORRECT
static void my_stop(struct seq_file *m, void *v) {
    spin_unlock(&my_lock);
}
```

**3. Buffer overflow with seq_printf:**

```c
// WRONG - Can overflow seq buffer
char huge_buffer[10000];
seq_printf(m, "%s", huge_buffer);  // Don't do this

// CORRECT - seq_printf handles it
seq_printf(m, "%s", data);
```

## 16. Using `seq_file` with sysfs

### What is sysfs?

`sysfs` is a virtual filesystem mounted at `/sys` that exposes kernel objects and their attributes. Unlike `/proc`, `sysfs` is more structured and attribute-oriented.

### When to Use `seq_file` with sysfs

Use `seq_file` in sysfs when:

* Attribute values contain multiple lines
* You need formatted output (similar to procfs)
* Reading requires complex calculations
* Output can be large

### Basic sysfs Attribute with seq_file

```c
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/seq_file.h>

static int device_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Device Status Information\n");
    seq_printf(m, "Temperature: 45°C\n");
    seq_printf(m, "Voltage: 5V\n");
    seq_printf(m, "Current: 1.2A\n");
    return 0;
}

static ssize_t device_seq_read(struct file *file, char __user *buf,
                               size_t size, loff_t *ppos)
{
    struct seq_file *m = file->private_data;
    return seq_read(file, buf, size, ppos);
}

static int device_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, device_show, inode->i_private);
}

static const struct file_operations device_seq_fops = {
    .open    = device_seq_open,
    .read    = device_seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

// In device attribute
static ssize_t device_status_show(struct device *dev, 
                                   struct device_attribute *attr, 
                                   char *buf)
{
    // For simple attributes, direct sprintf is fine
    return sprintf(buf, "Device Status: ONLINE\n");
}

// For multi-line output, use seq_file approach
static struct bin_attribute device_info_attr = {
    .attr = {
        .name = "info",
        .mode = 0444,
    },
    .fops = &device_seq_fops,
    .size = PAGE_SIZE,
};
```

### sysfs Group with seq_file Example

```c
#include <linux/device.h>
#include <linux/sysfs.h>

struct my_device {
    struct device dev;
    int temperature;
    int humidity;
    char status[64];
};

static int sensor_info_show(struct seq_file *m, void *v)
{
    struct my_device *dev = (struct my_device *)m->private;
    
    seq_printf(m, "Sensor Information\n");
    seq_printf(m, "==================\n");
    seq_printf(m, "Status: %s\n", dev->status);
    seq_printf(m, "Temperature: %d°C\n", dev->temperature);
    seq_printf(m, "Humidity: %d%%\n", dev->humidity);
    seq_printf(m, "Last Updated: %lld\n", ktime_get_ns());
    
    return 0;
}

static int sensor_info_open(struct inode *inode, struct file *file)
{
    struct my_device *dev = inode->i_private;
    return single_open(file, sensor_info_show, dev);
}

static const struct file_operations sensor_info_fops = {
    .open    = sensor_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

// Registration in device probe
static int my_device_probe(struct platform_device *pdev)
{
    struct my_device *dev;
    struct dentry *dentry;
    
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;
    
    dev_set_drvdata(&pdev->dev, dev);
    
    // Create sysfs directory
    dev->dev.parent = &pdev->dev;
    device_register(&dev->dev);
    
    // Create binary attribute with seq_file
    dentry = debugfs_create_file("info", 0444, NULL, dev, &sensor_info_fops);
    if (!dentry)
        goto error;
    
    return 0;

error:
    kfree(dev);
    return -ENOMEM;
}
```

### Comparison: procfs vs sysfs with seq_file

| Aspect            | procfs           | sysfs                  |
| ----------------- | ---------------- | ---------------------- |
| Mount Point       | `/proc`          | `/sys`                 |
| Purpose           | Process info     | Device/attribute info  |
| Structure         | Flat/hierarchical| Hierarchical (kobjects)|
| Typical Use       | Diagnostics      | Device control         |
| single_open()     | Common           | Sometimes used         |
| Iterator pattern  | For lists        | Less common            |
| File size         | Unbounded        | Usually small (PAGE)   |

### Key Differences

1. **sysfs attributes are tied to kobjects** - Each file must be associated with a device/kobject
2. **sysfs uses binary attributes** - More restricted API than procfs
3. **sysfs is attribute-based** - Designed for single-value or multi-line attributes
4. **Read-only typically** - Most sysfs files are read-only

### Complete sysfs Module with seq_file

```c
#include <linux/module.h>
#include <linux/device.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("sysfs seq_file example");

struct test_device {
    struct device dev;
    int counter;
    char name[32];
};

static struct test_device *test_dev;

static int test_info_show(struct seq_file *m, void *v)
{
    struct test_device *dev = (struct test_device *)m->private;
    
    seq_printf(m, "Test Device Info\n");
    seq_printf(m, "Name: %s\n", dev->name);
    seq_printf(m, "Counter: %d\n", dev->counter);
    seq_printf(m, "Reads: %d\n", ++dev->counter);
    
    return 0;
}

static int test_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, test_info_show, test_dev);
}

static const struct file_operations test_info_fops = {
    .open    = test_info_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static ssize_t test_name_show(struct device *dev, 
                              struct device_attribute *attr,
                              char *buf)
{
    struct test_device *tdev = dev_get_drvdata(dev);
    return sprintf(buf, "%s\n", tdev->name);
}

static ssize_t test_name_store(struct device *dev,
                               struct device_attribute *attr,
                               const char *buf, size_t count)
{
    struct test_device *tdev = dev_get_drvdata(dev);
    sscanf(buf, "%31s", tdev->name);
    return count;
}

static DEVICE_ATTR(name, 0644, test_name_show, test_name_store);

static int __init test_sysfs_init(void)
{
    int ret;
    
    // Create device
    test_dev = kzalloc(sizeof(*test_dev), GFP_KERNEL);
    if (!test_dev)
        return -ENOMEM;
    
    strcpy(test_dev->name, "TestDevice");
    
    // Register device
    ret = device_register(&test_dev->dev);
    if (ret) {
        kfree(test_dev);
        return ret;
    }
    
    // Add simple attribute
    device_create_file(&test_dev->dev, &dev_attr_name);
    
    // Add seq_file based attribute via debugfs alternative
    // (Note: sysfs itself doesn't have a direct seq_file API)
    
    printk(KERN_INFO "Test sysfs device created at /sys/devices/virtual/misc/test\n");
    
    return 0;
}

static void __exit test_sysfs_exit(void)
{
    device_remove_file(&test_dev->dev, &dev_attr_name);
    device_unregister(&test_dev->dev);
    kfree(test_dev);
}

module_init(test_sysfs_init);
module_exit(test_sysfs_exit);
```

### Using seq_file with debugfs (procfs-like interface under /sys/kernel/debug)

For larger, more complex output in the `/sys` hierarchy, use **debugfs** instead:

```c
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int debug_status_show(struct seq_file *m, void *v)
{
    seq_printf(m, "Debug Information\n");
    seq_printf(m, "=================\n");
    seq_printf(m, "Kernel Debug Mode: ON\n");
    seq_printf(m, "Build: Debug\n");
    
    return 0;
}

static int debug_status_open(struct inode *inode, struct file *file)
{
    return single_open(file, debug_status_show, NULL);
}

static const struct file_operations debug_fops = {
    .open    = debug_status_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

// In module init
static struct dentry *debug_dir;

static int __init debug_module_init(void)
{
    debug_dir = debugfs_create_dir("mydevice", NULL);
    if (!debug_dir)
        return -ENOMEM;
    
    debugfs_create_file("status", 0444, debug_dir, NULL, &debug_fops);
    
    // Now available at: /sys/kernel/debug/mydevice/status
    return 0;
}

// In module exit
static void __exit debug_module_exit(void)
{
    debugfs_remove_recursive(debug_dir);
}
```

---

## 17. Further Study Topics

* How `/proc/modules` uses `seq_file`
* How `/proc/meminfo` uses `seq_file`
* Locking within seq iterators
* RCU-safe `seq_file` iteration
* Combining `seq_file` with debugfs
* Using `seq_file` with cdev and character devices
* sysfs attributes and device models
* debugfs for debugging output
