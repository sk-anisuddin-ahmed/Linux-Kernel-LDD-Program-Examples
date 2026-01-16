# Synchronization Primitives in Linux Kernel Drivers

## 1. PER-CPU VARIABLE APPLICATION

### (High-frequency sensor event counting)

#### Problem

- Sensor generates very frequent events
- Multiple CPUs handle interrupts
- Using a single global counter causes:
  - Cache bouncing
  - Heavy contention
  - Poor performance

#### Solution

- Use per-CPU variables:

  ```c
  DEFINE_PER_CPU(struct sensor_cnt, sensor_data);
  ```

- Each CPU:
  - Updates its own local counter
  - No locking needed
  - No cache-line sharing

#### Flow

- IRQ on CPU0 → update CPU0 counter
- IRQ on CPU1 → update CPU1 counter
- User read → aggregate all CPUs

#### Why per-CPU is best

- Lock-free
- Scales with CPUs
- Ideal for statistics, counters

#### Real usage

- Network packet counters
- Scheduler stats
- Interrupt statistics

#### Code Example

```c
#include <linux/percpu.h>
#include <linux/interrupt.h>

struct sensor_cnt {
    unsigned long events;
};

DEFINE_PER_CPU(struct sensor_cnt, sensor_data);

static irqreturn_t sensor_isr(int irq, void *dev_id)
{
    this_cpu_inc(sensor_data.events);
    return IRQ_HANDLED;
}

static ssize_t sensor_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long total = 0;
    int cpu;
    for_each_possible_cpu(cpu) {
        total += per_cpu(sensor_data.events, cpu);
    }
    return simple_read_from_buffer(buf, count, ppos, &total, sizeof(total));
}
```

## 2. ATOMIC APPLICATION

### (Embedded sensor power & user count)

#### Problem

- Multiple user apps open the device
- Sensor must:
  - Power ON when first user opens
  - Power OFF when last user closes
- Race condition possible

#### Solution

- Use `atomic_t`:

  ```c
  atomic_t users;
  ```

#### Flow

- `open()`:
  - `users++`
  - if `users == 1` → power ON
- `close()`:
  - `users--`
  - if `users == 0` → power OFF

#### Why atomic is correct

- Simple integer state
- No sleeping
- Lock-free & fast

#### Why not mutex/semaphore

- Overkill
- Extra overhead
- Atomic is sufficient

#### Real usage

- Power management
- Reference counting
- Open-close tracking

#### Code Example

```c
#include <linux/atomic.h>
#include <linux/fs.h>

static atomic_t users = ATOMIC_INIT(0);

static int sensor_open(struct inode *inode, struct file *file)
{
    if (atomic_inc_return(&users) == 1) {
        /* Power ON sensor */
        pr_info("Sensor powered ON\n");
    }
    return 0;
}

static int sensor_release(struct inode *inode, struct file *file)
{
    if (atomic_dec_return(&users) == 0) {
        /* Power OFF sensor */
        pr_info("Sensor powered OFF\n");
    }
    return 0;
}
```

## 3. SEMAPHORE APPLICATION

### (ADC with limited hardware slots)

#### Problem

- ADC supports only 2 conversions at a time
- More user apps may request ADC
- Extra users must wait, not fail

#### Solution

- Use counting semaphore:

  ```c
  sema_init(&adc_sem, 2);
  ```

#### Flow

- `open()`:
  - `down(adc_sem)` → blocks if full
- `read()`:
  - ADC conversion
- `close()`:
  - `up(adc_sem)` → wakes next process

#### Why semaphore is correct

- Limits concurrent access
- Allows sleeping
- Models hardware slots naturally

#### Why mutex is wrong here

- Mutex allows only 1
- Hardware allows 2
- Semaphore models count

#### Real usage

- ADC engines
- DMA channels
- Shared buses

#### Code Example

```c
#include <linux/semaphore.h>
#include <linux/fs.h>

static struct semaphore adc_sem;

static int __init adc_init(void)
{
    sema_init(&adc_sem, 2);  /* Allow 2 concurrent accesses */
    return 0;
}

static int adc_open(struct inode *inode, struct file *file)
{
    if (down_interruptible(&adc_sem)) {
        return -ERESTARTSYS;
    }
    return 0;
}

static int adc_release(struct inode *inode, struct file *file)
{
    up(&adc_sem);
    return 0;
}

static ssize_t adc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    /* Perform ADC conversion here */
    /* ... */
    return count;
}
```

## 4. MUTEX APPLICATION

### (Exclusive sensor configuration)

#### Problem

- Sensor calibration/config must be:
  - Exclusive
  - Sleepable
  - Only one process allowed at a time

#### Solution

- Use mutex:

  ```c
  mutex_lock(&sensor_lock);
  ```

#### Flow

- `ioctl()`:
  - `mutex_lock`
  - calibration
  - `mutex_unlock`

#### Why mutex is correct

- Sleep allowed
- Strict exclusivity
- Simple semantics

#### Why semaphore is weaker

- Semaphore >1 possible
- Mutex enforces ownership

#### Real usage

- Sensor calibration
- Device configuration
- Power sequencing

#### Code Example

```c
#include <linux/mutex.h>
#include <linux/fs.h>

static DEFINE_MUTEX(sensor_lock);

static long sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    mutex_lock(&sensor_lock);
    switch (cmd) {
    case SENSOR_CALIBRATE:
        /* Perform calibration */
        pr_info("Calibrating sensor\n");
        break;
    default:
        ret = -EINVAL;
    }
    mutex_unlock(&sensor_lock);
    return ret;
}
```

## 5. SPINLOCK APPLICATION

### (IRQ + user-space statistics)

#### Problem

- Data updated in interrupt context
- Same data read in process context
- Sleeping is forbidden in IRQ

#### Solution

- Use spinlock:

  ```c
  spin_lock_irqsave(&lock, flags);
  ```

#### Flow

- IRQ:
  - `spin_lock`
  - update stats
  - `spin_unlock`
- `read()`:
  - `spin_lock`
  - copy stats
  - `spin_unlock`

#### Why ONLY spinlock works

| Primitive | Valid in IRQ |
|-----------|--------------|
| Mutex     | ❌          |
| Semaphore | ❌          |
| Atomic    | ❌ (multi-field) |
| Spinlock  | ✅          |

#### Real usage

- Network drivers
- CAN drivers
- UART RX/TX stats

#### Code Example

```c
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/fs.h>

static spinlock_t stats_lock;
static unsigned long rx_count, tx_count;

static irqreturn_t uart_isr(int irq, void *dev_id)
{
    unsigned long flags;
    spin_lock_irqsave(&stats_lock, flags);
    rx_count++;
    spin_unlock_irqrestore(&stats_lock, flags);
    return IRQ_HANDLED;
}

static ssize_t stats_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long flags;
    char stats[64];
    int len;
    spin_lock_irqsave(&stats_lock, flags);
    len = snprintf(stats, sizeof(stats), "RX: %lu, TX: %lu\n", rx_count, tx_count);
    spin_unlock_irqrestore(&stats_lock, flags);
    return simple_read_from_buffer(buf, count, ppos, stats, len);
}
```

## 6. WORKQUEUE APPLICATION

### (Temperature sensor background processing)

#### Problem

- Sensor read is slow
- Sleep required (I2C/SPI)
- Must not run in IRQ or fast path
- Must be periodic

#### Solution

- Use delayed workqueue:

  ```c
  queue_delayed_work()
  ```

#### Flow

- module load →
  - schedule work ONCE
- workqueue:
  - read sensor (sleep)
  - update cached value
  - reschedule itself
- user read:
  - return cached value

#### Why workqueue is perfect

- Runs in process context
- Sleep allowed
- Deferred execution
- Clean separation

#### Why timer/tasklet wrong

- Timer/tasklet run in atomic context
- Cannot sleep

#### Real usage

- Thermal sensors
- hwmon drivers
- IIO buffered sensors

#### Code Example

```c
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/fs.h>

static struct delayed_work temp_work;
static int cached_temp;

static void temp_work_func(struct work_struct *work)
{
    /* Simulate slow sensor read with sleep */
    msleep(100);
    cached_temp = 25;  /* Read actual temp here */
    queue_delayed_work(system_wq, &temp_work, HZ);  /* Reschedule every 1s */
}

static int __init temp_init(void)
{
    INIT_DELAYED_WORK(&temp_work, temp_work_func);
    queue_delayed_work(system_wq, &temp_work, 0);
    return 0;
}

static ssize_t temp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(buf, count, ppos, &cached_temp, sizeof(cached_temp));
}
```

## Additional Missing Information

- **Completion Variables**: For synchronization between threads, e.g., waiting for work to finish.
- **RCU (Read-Copy-Update)**: For lock-free reads in multi-writer scenarios.
- **Seqlocks**: For readers that can retry on writer interference.
- **Futexes**: User-space synchronization primitives.
- **Lockdep**: Kernel debugging tool for detecting lock ordering issues.
- **Memory Barriers**: Ensuring order of operations across CPUs.

These can be added as additional sections if needed for completeness.
