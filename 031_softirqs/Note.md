# softirq2 - Custom Softirq Vector Guide

This module only raises `MY_SOFTIRQ`. The handler must be registered in the kernel.

## Where to add `open_softirq()`

Add the handler registration in the kernel tree in `softirq_init()` inside:
- `kernel/softirq.c`

Also add the vector ID to:
- `include/linux/interrupt.h`

## Build
1) Apply the patch in kernel source.
2) Rebuild and boot the new kernel.
3) Load the module and check `dmesg`.

## Notes
- `open_softirq()` is not exported to modules, so it must live in the kernel tree.
- The handler runs in softirq context and must not sleep.
