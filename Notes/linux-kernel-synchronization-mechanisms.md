# LINUX KERNEL SYNCHRONIZATION MECHANISMS

**rwlock, rw_semaphore, seqlock, and RCU**

---

## INTRODUCTION

Modern multiprocessor systems execute multiple threads on multiple CPUs simultaneously.
This leads to concurrent access to shared kernel data.

Linux provides different synchronization primitives based on:

- Execution context (process / interrupt)
- Read vs write frequency
- Performance requirements
- Whether sleeping is allowed

---

## 1. READ–WRITE SPINLOCK (rwlock_t)

### WHY rwlock WAS INTRODUCED

- Normal spinlock allows only one CPU at a time, even for reads.
- Many kernel data structures are:
  - Read very frequently
  - Written rarely
- Using spinlock here reduces parallelism.

### CORE IDEA

- Allow multiple readers simultaneously.
- Allow only one writer.
- No readers allowed during writing.

### INTERNAL WORKING

- Maintains:
  - Reader count
  - Writer ownership flag
- Uses busy waiting (spinning).

**Reader behavior:**

- Check if writer is active
- Increment reader count
- Enter critical section
- Decrement reader count on exit

**Writer behavior:**

- Spin until no readers and no writers
- Acquire lock exclusively
- Modify shared data
- Release lock

### KEY CHARACTERISTICS

- Fast
- Interrupt-safe
- Cannot sleep
- Writer starvation possible

### EXAMPLE CODE

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rwlock.h>

rwlock_t mylock;
int shared_data = 0;

static int __init rwlock_demo_init(void)
{
    rwlock_init(&mylock);

    /* Writer section */
    write_lock(&mylock);
    shared_data += 10;
    write_unlock(&mylock);

    /* Reader section */
    read_lock(&mylock);
    printk(KERN_INFO "rwlock read value = %d\n", shared_data);
    read_unlock(&mylock);

    return 0;
}

static void __exit rwlock_demo_exit(void)
{
    printk(KERN_INFO "rwlock module exit\n");
}

module_init(rwlock_demo_init);
module_exit(rwlock_demo_exit);
MODULE_LICENSE("GPL");
```

### WHERE rwlock IS USED

- Scheduler data
- IRQ-safe kernel paths
- Small critical sections

### MENTAL MODEL
>
> "Many people can read a notice board, but when someone updates it, nobody else is allowed."

---

## 2. READ–WRITE SEMAPHORE (rw_semaphore)

### WHY rw_semaphore EXISTS

- rwlock spins and wastes CPU time for long operations.
- Not suitable if code may sleep or access disk.

### CORE IDEA

- Same reader-writer concept as rwlock.
- Threads sleep instead of spinning.

### INTERNAL WORKING

- Uses wait queues and scheduler.

**Reader:**

- Enter if no writer
- Sleep if writer exists

**Writer:**

- Sleeps until all readers exit

### KEY CHARACTERISTICS

- Sleeping allowed
- Efficient CPU usage
- Not interrupt-safe
- Slower than rwlock

### EXAMPLE CODE

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rwsem.h>

struct rw_semaphore mysem;
int data = 100;

static int __init rwsem_demo_init(void)
{
    init_rwsem(&mysem);

    /* Reader */
    down_read(&mysem);
    printk(KERN_INFO "rwsem read data = %d\n", data);
    up_read(&mysem);

    /* Writer */
    down_write(&mysem);
    data += 50;
    up_write(&mysem);

    return 0;
}

static void __exit rwsem_demo_exit(void)
{
    printk(KERN_INFO "rwsem module exit\n");
}

module_init(rwsem_demo_init);
module_exit(rwsem_demo_exit);
MODULE_LICENSE("GPL");
```

### WHERE rw_semaphore IS USED

- Filesystems
- Memory management (VMAs)
- Driver subsystems

### MENTAL MODEL
>
> "People sit and wait on chairs, not run in circles."

---

## 3. SEQUENCE LOCK (seqlock_t)

### PROBLEM seqlock SOLVES

- rwlock blocks readers or writers.
- Suitable when readers are very frequent and writers are rare.

### CORE IDEA

- Readers do not lock.
- Readers retry if a write occurs.

### INTERNAL MECHANISM

- Uses a sequence counter.
  - Even value: no writer active
  - Odd value: writer active

**Writer:**

- Increment sequence (even to odd)
- Modify data
- Increment sequence (odd to even)

**Reader:**

- Read sequence
- Read data
- Read sequence again
- Retry if changed

### EXAMPLE CODE

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/seqlock.h>

seqlock_t seq_lock;
int counter = 0;

static int __init seqlock_demo_init(void)
{
    unsigned seq;
    int val;

    seqlock_init(&seq_lock);

    /* Writer */
    write_seqlock(&seq_lock);
    counter += 1;
    write_sequnlock(&seq_lock);

    /* Reader */
    do {
        seq = read_seqbegin(&seq_lock);
        val = counter;
    } while (read_seqretry(&seq_lock, seq));

    printk(KERN_INFO "seqlock read value = %d\n", val);

    return 0;
}

static void __exit seqlock_demo_exit(void)
{
    printk(KERN_INFO "seqlock module exit\n");
}

module_init(seqlock_demo_init);
module_exit(seqlock_demo_exit);
MODULE_LICENSE("GPL");
```

### WHY seqlock IS FAST

- Readers never block
- No sleeping
- No cache bouncing

### LIMITATIONS

- Reader starvation possible
- Not suitable for large data
- Writers are exclusive

### WHERE seqlock IS USED

- Timekeeping
- Statistics
- Kernel clocks

### MENTAL MODEL
>
> "If someone edits the paper while you read, start reading again."

---

## 4. RCU (READ–COPY–UPDATE)

### WHY RCU EXISTS

- Even seqlock retries can hurt performance.
- Designed for near-zero reader overhead.

### CORE IDEA

- Readers use old data safely.
- Writers create new copies.

### HOW RCU WORKS

**Reader:**

- Enter RCU read section
- Dereference pointer
- Exit read section
- No locking or spinning

**Writer:**

- Allocate new object
- Update pointer atomically
- Wait for grace period
- Free old object

**GRACE PERIOD**

- Time until all existing readers finish.
- Kernel tracks context switches and CPU states.

### EXAMPLE CODE

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>

struct mydata {
    int value;
};

struct mydata *gp;

static int __init rcu_demo_init(void)
{
    struct mydata *new;
    struct mydata *p;

    /* Writer */
    new = kmalloc(sizeof(*new), GFP_KERNEL);
    new->value = 999;

    rcu_assign_pointer(gp, new);

    /* Reader */
    rcu_read_lock();
    p = rcu_dereference(gp);
    if (p)
        printk(KERN_INFO "RCU read value = %d\n", p->value);
    rcu_read_unlock();

    synchronize_rcu();
    return 0;
}

static void __exit rcu_demo_exit(void)
{
    kfree(gp);
    printk(KERN_INFO "RCU module exit\n");
}

module_init(rcu_demo_init);
module_exit(rcu_demo_exit);
MODULE_LICENSE("GPL");
```

### WHY RCU IS FASTEST

- Readers are wait-free
- No atomic operations in read path
- Excellent SMP scalability

### WHERE RCU IS USED

- Process lists
- Networking stack
- Routing tables
- VFS

### MENTAL MODEL
>
> "Readers finish the old newspaper, writers print the new edition."

---

## FINAL COMPARISON

| Mechanism | Use Case |
|-----------|----------|
| **rwlock** | Short, IRQ-safe critical sections |
| **rw_semaphore** | Long operations with sleeping |
| **seqlock** | Statistics and clocks |
| **RCU** | Massive read-heavy workloads |

---

## MEMORY TRICK

| Mechanism | Pattern |
|-----------|---------|
| **rwlock** | spin & share |
| **rw_semaphore** | sleep & share |
| **seqlock** | retry on conflict |
| **RCU** | read old, write new |
