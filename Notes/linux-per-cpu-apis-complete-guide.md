# Linux Per-CPU APIs Notes

## 1. What is a Per-CPU Variable?

A **per-CPU variable** provides **one private instance per CPU**.

Why it exists:
- Avoids locks
- Avoids atomics
- Avoids cache-line bouncing

Each CPU updates its own copy → **excellent SMP scalability**.

Used heavily in:
- Scheduler (runqueues)
- Networking (stats)
- IRQ / SoftIRQ
- Memory allocator statistics

---

## 2. Required Kernel Headers

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/percpu.h>
#include <linux/smp.h>
#include <linux/preempt.h>
```

---

## 3. Declaration & Allocation APIs

### DEFINE_PER_CPU
- **Type**: Macro  
- **Prototype**: `DEFINE_PER_CPU(type, name);`  
- **Return**: N/A  
- **Description**: Defines a static per-CPU variable at compile time.

```c
DEFINE_PER_CPU(int, cpu_counter);
```

---

### DECLARE_PER_CPU
- **Type**: Macro  
- **Prototype**: `DECLARE_PER_CPU(type, name);`  
- **Return**: N/A  
- **Description**: Extern declaration for per-CPU variable defined elsewhere.

---

### alloc_percpu / free_percpu
- **Prototype**:
```c
void __percpu *alloc_percpu(type);
void free_percpu(void __percpu *ptr);
```
- **Return**: per-CPU pointer / void  
- **Description**: Runtime allocation of per-CPU memory.

```c
int __percpu *dyn;
dyn = alloc_percpu(int);
free_percpu(dyn);
```

---

## 4. Core Access APIs

### per_cpu()
- **Type**: Macro  
- **Prototype**: `per_cpu(var, cpu)`  
- **Return**: lvalue of `var`  
- **Description**: Access per-CPU variable of a specific CPU.
- ⚠ Unsafe if task migrates

---

### this_cpu_* (Fast Path APIs)

- **Prototypes**:
```c
this_cpu_read(var)
this_cpu_write(var, val)
this_cpu_inc(var)
this_cpu_dec(var)
this_cpu_add(var, n)
this_cpu_sub(var, n)
```
- **Return**: value / void  
- **Description**: Access current CPU copy **without disabling preemption**.

```c
this_cpu_inc(cpu_counter);
```

---

## 5. Safe Access APIs (Disable Preemption)

### get_cpu_var / put_cpu_var
- **Type**: Macro  
- **Prototype**: `get_cpu_var(var)` / `put_cpu_var(var)`  
- **Return**: lvalue / void  
- **Description**: Disables preemption to guarantee CPU consistency.

```c
get_cpu_var(cpu_counter)++;
put_cpu_var(cpu_counter);
```

---

### get_cpu / put_cpu
- **Prototype**:
```c
int get_cpu(void);
void put_cpu(void);
```
- **Return**: CPU id / void  
- **Description**: Explicit CPU pinning for critical sections.

---

## 6. Pointer-Based APIs (Dynamic Per-CPU)

### per_cpu_ptr / this_cpu_ptr
- **Prototypes**:
```c
void *per_cpu_ptr(void __percpu *ptr, int cpu);
void *this_cpu_ptr(void __percpu *ptr);
```
- **Return**: CPU-specific pointer  
- **Description**: Access dynamic per-CPU memory.

```c
*per_cpu_ptr(dyn, cpu) = 10;
```

---

## 7. CPU Iteration Macros

| Macro | Description |
|------|------------|
| `for_each_possible_cpu(cpu)` | All possible CPUs |
| `for_each_present_cpu(cpu)` | Physically present CPUs |
| `for_each_online_cpu(cpu)` | Online CPUs (most common) |

```c
for_each_online_cpu(cpu)
    pr_info("CPU %d val=%d\n", cpu, per_cpu(cpu_counter, cpu));
```

---

## 8. Execute Code on CPUs

### on_each_cpu()
- **Prototype**:
```c
int on_each_cpu(void (*fn)(void *), void *info, int wait);
```
- **Return**: 0 on success  
- **Description**: Runs function on **all online CPUs**.

```c
static void cpu_fn(void *info)
{
    this_cpu_inc(cpu_counter);
}

on_each_cpu(cpu_fn, NULL, 1);
```

---

## 9. COMPLETE Kernel Module Example (Static Per-CPU)

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/percpu.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("example");
MODULE_DESCRIPTION("Per-CPU static example");

DEFINE_PER_CPU(int, counter);

static int __init percpu_init(void)
{
    int cpu;

    /* Initialize per-CPU variables */
    for_each_online_cpu(cpu)
        per_cpu(counter, cpu) = cpu * 10;

    /* Safe update on current CPU */
    get_cpu_var(counter) += 5;
    put_cpu_var(counter);

    /* Print values */
    for_each_online_cpu(cpu)
        pr_info("CPU %d counter=%d\n", cpu, per_cpu(counter, cpu));

    return 0;
}

static void __exit percpu_exit(void)
{
    pr_info("Per-CPU module exit\n");
}

module_init(percpu_init);
module_exit(percpu_exit);
```

---

## 10. Kernel Module Example (Dynamic Per-CPU)

```c
static int __percpu *dyn;

static int __init dyn_init(void)
{
    int cpu;

    dyn = alloc_percpu(int);
    if (!dyn)
        return -ENOMEM;

    for_each_online_cpu(cpu)
        *per_cpu_ptr(dyn, cpu) = cpu * 100;

    return 0;
}

static void __exit dyn_exit(void)
{
    free_percpu(dyn);
}
```

---

## 11. Per-CPU vs Atomic vs Spinlock

| Method | Lock | Cache Bounce | Speed |
|------|------|--------------|-------|
| Spinlock | Yes | High | Slow |
| Atomic | No | High | Medium |
| Per-CPU | No | None | Fast |

---

## 12. Context Safety

| Context | Safe APIs |
|-------|-----------|
| Process | All APIs |
| SoftIRQ | this_cpu_* |
| IRQ | this_cpu_* |
| Preemptible | Avoid per_cpu() |

---

## 13. Must-Remember Points

- Per-CPU data usually needs **no locks**
- Unsafe access breaks if CPU migrates
- Disable preemption when consistency matters
- Prefer per-CPU for counters & stats
- Used extensively in core kernel subsystems

