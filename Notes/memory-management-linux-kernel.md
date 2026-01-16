# Linux Kernel Memory Management

## MEMORY FUNDAMENTALS

### 0. Memory Management Unit (MMU) - The Core Concept

The **MMU** is a specialized hardware component inside the CPU that handles address translation. It is the foundation of modern memory management and makes virtual memory possible.

#### 0.1 What is the MMU?

The MMU translates virtual addresses (used by programs) into physical addresses (actual hardware memory locations) in real-time.

```
Program requests memory at:  0x08048000 (virtual address)
        ↓↓↓ MMU Translation
Hardware accesses memory at: 0x20001234 (physical address)
```

| Scenario | Without MMU | With MMU |
|----------|-------------|----------|
| **Address type** | Real hardware addresses | Virtual addresses |
| **Isolation** | None | Complete isolation per process |
| **Security** | Crashes affect entire system | Protected address spaces |
| **Flexibility** | Fixed memory layout | Can remap/relocate/swap |

#### 0.2 Physical vs Virtual Addresses

| Concept | Physical Address | Virtual Address |
|---------|------------------|-----------------|
| **What is it** | Real location in RAM/device | Fake address seen by program |
| **Who uses it** | CPU hardware, device controllers | Programs and kernel |
| **Unique per** | System (fixed) | Process (each has own mapping) |
| **Range** | 0 to 2^(address bus width) | 0 to 2^(address bus width) |
| **Example** | 0x20001000 (real DDR3 chip) | 0x08048000 (appears to program) |
| **Benefit** | Direct hardware control | Isolation and protection |

**Key Insight**: Two programs can both use virtual address `0x08048000`, but they access completely different physical RAM locations.

#### 0.3 How MMU Translation Works

**Step-by-Step Process**:

```
1. Program executes: mov rax, [0x08048000]    ; Load from virtual address
   ↓
2. CPU issues request to MMU: "Translate 0x08048000"
   ↓
3. MMU looks up Page Table: Virtual 0x08048000 → Physical 0x20001234
   ↓
4. MMU returns physical address: 0x20001234
   ↓
5. CPU accesses RAM at 0x20001234 and gets the data
   ↓
6. Program receives value without knowing it accessed 0x20001234
```

**Page Table**: Kernel-maintained lookup table that tells MMU which virtual pages map to which physical pages.

#### 0.4 MMU Hardware Internals (Simplified)

```c
// Simplified MMU operation (pseudocode)
PhysicalAddress mmu_translate(VirtualAddress vaddr) {
    // Extract page number from virtual address
    int page_num = vaddr >> 12;  // Divide by 4096 (12-bit shift)
    
    // Look up in page table
    PageTableEntry pte = page_table[page_num];
    
    // Extract physical page number and combine with offset
    PhysicalAddress paddr = (pte.phys_page_num << 12) | (vaddr & 0xFFF);
    
    return paddr;  // Return to CPU
}
```

**Key Components**:

| Component | Purpose |
|-----------|---------|
| **Page Table** | Kernel data structure mapping virtual → physical pages |
| **TLB (Translation Lookaside Buffer)** | Cache of recent translations (very fast) |
| **Page Size** | 4 KB on ARM (offset = 12 bits = 0xFFF) |
| **Protection Bits** | Read/Write/Execute flags in page table entry |

#### 0.5 TLB (Translation Lookaside Buffer)

MMU includes a small but extremely fast cache called the TLB.

```
First access to 0x08048000:
  MMU: "Let me look this up..." → Page table lookup (SLOW)
       → Add to TLB cache
       → Return physical address

Second access to 0x08048000:
  MMU: "I remember this!" → TLB hit (VERY FAST)
       → Return cached physical address
```

**TLB Size**: Typically 32-1024 entries (varies by CPU)

**TLB Invalidation**: When page mappings change, TLB must be flushed (expensive operation)

#### 0.6 ARM MMU (BeagleBone Black, Raspberry Pi)

| Feature | ARMv7 (32-bit) | ARMv8 (64-bit) |
|---------|----------------|----------------|
| **Page Size** | 4 KB | 4 KB or 16 KB |
| **Page Tables** | Two-level | Three-level |
| **Address Space** | 4 GB (32-bit) | 256 TB (48-bit used) |
| **Protection** | User/Kernel modes | 4 exception levels (EL0-EL3) |
| **Use Case** | BeagleBone Black, RPi 3 | Raspberry Pi 4, modern servers |

#### 0.7 Enabling/Disabling MMU

| State | Characteristics | Timing | Used For |
|-------|-----------------|--------|----------|
| **MMU OFF** | 1:1 virtual-to-physical, no translation, direct hardware access | During boot | Bootloader initialization |
| **MMU ON** | Full address translation active, virtual addresses, per-process protection | After kernel init | All kernel and user execution |

**Code Example: ARM Boot Sequence**:

```c
// Early boot: MMU still OFF
void bootstrap_code(void) {
    // 0x20000000 is both virtual AND physical here
    uint32_t *ram = (uint32_t *)0x20000000;  // Direct access
    
    // Initialize page tables
    init_page_tables();
    
    // Now enable MMU
    enable_mmu();  // After this, all addresses become virtual
    
    // Now 0x20000000 is VIRTUAL address only
}
```

#### 0.8 Access Control & Protection

MMU enforces permissions for each page:

```
Page Table Entry includes:
- Physical address (where the data actually is)
- Present bit (is page in memory?)
- Read permission (can program read?)
- Write permission (can program write?)
- Execute permission (can program execute from here?)
- User/Kernel bit (who can access: user process or kernel only?)
```

**Example - Page Fault**:

```
Program tries: mov rax, [0x08048000]  ; Read from page
              ↓
MMU checks page table: Write protection set, trying to read ✓
                       Page present in memory ✓
                       User-mode process, user page ✓
              ↓
Access allowed → Data returned

Program tries: mov [0x08048000], rax  ; Write to page (read-only)
              ↓
MMU checks page table: Write protection set, trying to write ✗
              ↓
Page Fault exception → Kernel notified → Usually kill process
```

#### 0.9 MMU on ARM with Real Example (BeagleBone Black)

**Setup**:

```c
// Kernel initializes page tables mapping:
// Virtual 0x80000000 → Physical 0x20000000 (RAM)
// Virtual 0xC0000000 → Physical 0x20000000 (alternate kernel mapping)
// Virtual 0xF0000000 → Physical 0x44000000 (peripherals)

// User program gets separate mappings:
// Virtual 0x08048000 → Physical 0x20123000 (program's text)
// Virtual 0x08049000 → Physical 0x20124000 (program's data)
// Virtual 0x40000000 → Physical 0x20200000 (program's heap)
```

**Program Perspective**:

```c
// Program code
void *ptr = malloc(1024);      // Gets virtual address like 0x40000000
ptr[0] = 42;                   // Accesses via MMU

// Behind the scenes:
// CPU: "Translate 0x40000000"
// MMU: "That's 0x20200000 physical, allowed!"
// CPU: "Access 0x20200000 RAM" → Gets user data
```

#### 0.10 Why MMU Matters for Linux Kernel Development

| Aspect | Why Important | Example / Impact |
|--------|---------------|------------------|
| **Debugging** | Understand `/proc/[pid]/maps` output | Know which virtual addresses map to physical RAM |
| **Device Drivers** | Use `ioremap()` to safely map hardware | GPIO driver maps 0x44E00000 to kernel space |
| **Performance** | Optimize TLB usage and page walks | More TLB hits = fewer page table lookups |
| **Security** | Isolate processes from each other | One process crash doesn't affect others |
| **Embedded Systems** | Access hardware via virtual addresses | BeagleBone peripherals accessed safely |

---

### 1. Physical Address Space

#### 1.1 What is Physical Address Space?

Physical address space is the range of concrete hardware memory locations. Every byte in RAM and every device register has a unique physical address. The MMU (covered in Section 0) translates virtual addresses to physical ones.

---

#### 1.3 What's Inside the Address Space? (Components)

Memory address space divides into regions serving different purposes:

**Common Memory Components**:

| Component | Purpose | Notes |
|-----------|---------|-------|
| **RAM** | Program and OS storage | Main working memory |
| **ROM/Firmware** | Bootloader, initialization | Executes at power-on |
| **Device Registers** | GPIO, UART, timers, I2C, SPI | Control hardware via memory addresses |
| **Interrupt Controller** | Manage system interrupts | CPU communication |
| **Memory-mapped I/O** | Hardware control interface | Read/write = device operations |

**Memory-Mapped I/O Concept**:

| Aspect | Traditional I/O | Memory-Mapped I/O |
|--------|-----------------|-------------------|
| **Access Method** | Special I/O port instructions (IN/OUT) | Normal memory address (MOV, LOAD, STORE) |
| **Instruction Type** | Dedicated I/O opcodes | Standard memory opcodes |
| **Hardware Control** | Via port numbers | Via memory addresses |
| **Example** | GPIO control via port 0x60 | GPIO control via address 0x44E00000 |
| **ARM Support** | Limited/deprecated | Full native support |

---

#### 1.2 ARM Physical Address Layout (BeagleBone Black / AM335x SoC)

| Address Range | Component | Size | Purpose | Details |
|---|---|---|---|---|
| 0x20000000 - 0x3FFFFFFF | DDR3 RAM | 512 MB-1 GB | Main memory | Kernel loads at 0x20008000 |
| 0x44000000 - 0x48FFFFFF | OCP Peripherals | ~80 MB | All device registers | PRCM, GPIO, UART, I2C, SPI |
| 0x44C00000 | PRCM | 4 KB | Power/Clock Management | Module clock enables/disables |
| 0x44E00000 - 0x44E0FFFF | GPIO (0-3) | 64 KB | GPIO banks | GPIO0: LED, button control |
| 0x44E09000 | UART0 | 4 KB | Serial console | TX/RX for debug/communication |
| 0x44E0B000 | I2C0 | 4 KB | I2C bus 0 | Sensors, EEPROMs |
| 0x48000000 | SPI | 4 KB | SPI bus | SD card, external devices |
| 0xA0300000 | JTAG/Debug | 4 KB | Debugging interface | In-circuit emulation |

---

#### 1.3 Comparing ARM Embedded Boards

| Board | CPU | RAM | Key Peripherals | Use Case |
|-------|-----|-----|-----------------|----------|
| **BeagleBone Black** | ARM A8 (1 GHz) | 512 MB DDR3 | GPIO, UART, I2C, SPI, USB | Industrial applications, IoT |
| **Raspberry Pi 3** | ARM A53 (1.2 GHz) | 1 GB LPDDR2 | GPIO, UART, I2C, SPI | Education, media center |
| **Raspberry Pi 4** | ARM A72 (1.5 GHz) | 2-8 GB LPDDR4 | GPIO, USB3, Gigabit Ethernet | Development, performance |
| **NVIDIA Jetson Nano** | ARM A57 + GPU | 4 GB | GPIO, PCIe, USB3, Ethernet | AI/ML, robotics |

---

#### 1.4 Accessing Hardware Registers on ARM

**BeagleBone Black GPIO Example**:

```c
// Physical addresses (from datasheet)
#define GPIO0_BASE    0x44E00000
#define GPIO_DATAIN   0x38    // Read pins
#define GPIO_DATAOUT  0x3C    // Write pins

// Kernel module - proper way
void __iomem *gpio_virt = ioremap(GPIO0_BASE, 0x1000);
if (!gpio_virt) return -ENOMEM;

// Read GPIO inputs
uint32_t input_state = readl(gpio_virt + GPIO_DATAIN);

// Set GPIO output high
writel(0x00010000, gpio_virt + GPIO_DATAOUT);  // Set pin 16

iounmap(gpio_virt);  // Cleanup
```

**Common ARM Peripherals Registers**:

| Peripheral | Base Address | Register | Offset | Purpose | Operation |
|---|---|---|---|---|---|
| UART0 | 0x44E09000 | THR | +0x00 | Transmit holding | writel(char, addr) sends character |
| UART0 | 0x44E09000 | RBR | +0x00 | Receive buffer | readl(addr) gets incoming char |
| GPIO0 | 0x44E00000 | DATAOUT | +0x3C | Output data | writel(pin_mask, addr) sets pins high |
| GPIO0 | 0x44E00000 | DATAIN | +0x38 | Input data | readl(addr) reads GPIO pin states |
| GPIO0 | 0x44E00000 | OE | +0x34 | Output enable | 0=output, 1=input (per bit) |
| TIMER | 0x4A30003C | COUNT | +0x00 | Tick counter | readl(addr) gets current tick count |
| I2C0 | 0x44E0B000 | DATA | +0x1C | I2C data register | readl(addr) gets sensor value |

---

#### 1.5 View ARM Memory on Your System

**On BeagleBone Black or Raspberry Pi**:

```bash
# See physical memory layout
cat /proc/iomem

# Output example (BeagleBone):
20000000-3fffffff : System RAM     ← DDR3 memory
  20008000-209fffff : Kernel code
44000000-48ffffff : ocp            ← All peripherals
  44c00000-44c00fff : PRCM
  44e00000-44e00fff : GPIO
  44e09000-44e09fff : UART0
  44e0b000-44e0bfff : I2C0

# Memory usage
cat /proc/meminfo

# Actual kernel layout
cat /sys/kernel/debug/kernel_page_tables
```

---

#### 1.6 How Physical Addresses Map to Hardware (MMIO)

**Memory-Mapped I/O Behavior by Address**:

| Physical Address | Component | Access Type | Write Effect | Read Effect |
|---|---|---|---|---|
| 0x20001000 | RAM | Memory | Data stored | Data retrieved |
| 0x44E0B01C | I2C0 | Device | I2C transaction starts | Sensor reading returned |
| 0x44E09000 | UART0 THR | Device | Character sent to serial | Character received from serial |
| 0x44C0004C | PRCM Clock | Device | Module clock enabled/disabled | Clock status returned |
| 0x44E00000 | GPIO Data | Device | GPIO pins set high/low | GPIO pin states read |

This is the power of memory-mapped I/O on ARM—single address space for both RAM and hardware control.

---

---

#### 1.7 Practical: BeagleBone Black LED Blink

```c
#include <linux/module.h>
#include <linux/io.h>

#define GPIO0_BASE      0x44E00000
#define GPIO_OE         0x34        // Output Enable (0=output)
#define GPIO_DATAOUT    0x3C        // Data Out

// Set GPIO0[27] as output
void __iomem *gpio = ioremap(GPIO0_BASE, 0x1000);
uint32_t oe = readl(gpio + GPIO_OE);
writel(oe & ~(1 << 27), gpio + GPIO_OE);  // Set as output

// Toggle LED
#define LED_PIN 27
writel(1 << LED_PIN, gpio + GPIO_DATAOUT);   // LED on
mdelay(500);                                  // Wait
writel(0, gpio + GPIO_DATAOUT);             // LED off

iounmap(gpio);
MODULE_LICENSE("GPL");
```

---

### 2. Virtual Address Space

Virtual memory lets programs use fake addresses that the MMU translates to real locations. This provides isolation and protection.

#### 2.1 ARM 32-bit Virtual Memory Layout (3GB/1GB Split)

On BeagleBone Black and Raspberry Pi 3:

```
0xFFFFFFFF ┌─────────────────────────┐
           │ KERNEL SPACE (1 GB)     │ 0x80000000 - 0xFFFFFFFF (ARM)
           │ Kernel code, drivers,   │ 0xC0000000 - 0xFFFFFFFF (x86)
           │ device registers        │ 3GB/1GB split: CONFIG_PAGE_OFFSET
0x80000000 ├─────────────────────────┤ PAGE_OFFSET (ARM: 0x80000000, x86: 0xC0000000)
(ARM)      │ USER SPACE (3 GB on ARM)│ 0x00000000 - 0x7FFFFFFF (ARM)
OR         │ App code, heap, stack   │ 0x00000000 - 0xBFFFFFFF (x86)
0xC0000000 └─────────────────────────┘
(x86)
```

**Key Concept - PAGE_OFFSET**:

- ARM 32-bit: PAGE_OFFSET = 0x80000000 (3GB user / 1GB kernel)
- x86 32-bit: PAGE_OFFSET = 0xC0000000 (3GB user / 1GB kernel)
- Every process's virtual space includes kernel mapping for efficient syscalls (no address space switch needed)

#### 2.2 What's in Kernel Space?

| Address Range | Component | Size | Example |
|---|---|---|---|
| 0xFFFFFFFF - 0xF0000000 | Loaded drivers (modules) | 256 MB | Your insmod modules |
| 0xF0000000 - 0xE0000000 | Device I/O mappings | 256 MB | GPIO at 0xF0E00000 |
| 0xE0000000 - 0xC0000000 | Kernel memory (kmalloc) | 512 MB | Kernel heap |
| 0xC0000000 - 0x80000000 | Kernel RAM mapping | 1 GB | Maps to physical RAM |

**View current kernel layout**:

```bash
cat /proc/vmallocinfo    # See what's allocated where
cat /proc/modules        # Loaded drivers
dmesg | head -20         # Kernel memory at boot
```

#### 2.3 What's in User Space? (Your App)

Each application gets 2 GB private virtual space:

```
0x7FFFFFFF ┌──────────────┐
           │ STACK        │ (grows down ↓)
           │ local vars   │
           ├──────────────┤
           │ [GAP]        │ Must not collide!
           ├──────────────┤
           │ HEAP         │ (grows up ↑)
           │ malloc here  │
           ├──────────────┤
           │ BSS          │ Uninitialized globals
           ├──────────────┤
           │ DATA         │ Initialized globals
           ├──────────────┤
           │ TEXT (CODE)  │ Executable program
0x00000000 └──────────────┘
```

**View your app's memory**:

```bash
cat /proc/$(pidof myapp)/maps
```

**Common problem**: Stack and heap collide → crash!

#### 2.4 How Virtual Addresses Actually Work

**Simple example on BeagleBone**:

```c
int *ptr = malloc(100);    // Returns virtual address 0x40000000
*ptr = 42;                 // What happens?
```

**Behind the scenes**:

1. CPU says to MMU: "Translate virtual 0x40000000"
2. MMU looks up page table: "0x40000 → physical page 0x20200"
3. MMU calculates: physical address = 0x20200100
4. CPU accesses physical RAM at 0x20200100
5. Your data is safely written!

**Key insight**: Your program NEVER knows it's using physical address 0x20200100. It only knows 0x40000000.

#### 2.5 Accessing Hardware from Kernel (ioremap)

BeagleBone has GPIO hardware at physical address 0x44E00000. Kernel must map it:

```c
// Step 1: Map physical GPIO to virtual address
void __iomem *gpio = ioremap(0x44E00000, 0x1000);

// Step 2: Use virtual address to access hardware
writel(0x00010000, gpio + 0x3C);  // Set LED pin via virtual address

// Step 3: Cleanup
iounmap(gpio);
```

**What's happening**:

- ioremap creates mapping: physical 0x44E00000 → virtual 0xF0E00000
- Both user app and kernel can map same hardware to different virtual addresses
- Each gets their own safe virtual address

#### 2.6 Stack vs Heap Collision (Why Segfault)

On 32-bit ARM with limited 2 GB user space, stack and heap can meet:

```
Healthy:          Problem:
Stack ↓           Stack ↓ 
 |                |
 |                ↓ COLLISION!
 |                ↑
Heap ↑            Heap ↑
```

If gap becomes zero → "Segmentation fault"

**Prevention**: Don't use deep recursion, monitor heap size.

#### 2.7 Real Code Example

```c
#include <stdio.h>
#include <stdlib.h>

int global_counter = 100;       // In DATA segment
char buffer[1000] = {0};        // In BSS segment

int main(void) {
    int local_var = 42;         // On STACK
    int *heap_ptr = malloc(256); // HEAP allocation
    
    printf("TEXT (code):    0x%p\n", (void *)main);
    printf("DATA (global):  0x%p\n", (void *)&global_counter);
    printf("BSS (buffer):   0x%p\n", (void *)buffer);
    printf("HEAP (malloc):  0x%p\n", (void *)heap_ptr);
    printf("STACK (local):  0x%p\n", (void *)&local_var);
    
    free(heap_ptr);
    return 0;
}
```

**Run it**: You'll see addresses in different regions!

#### 2.8 Common Virtual Memory Problems

| Problem | Solution |
|---------|----------|
| **Segfault at random address** | Use gdb to find which pointer is bad |
| **Stack overflow from recursion** | Reduce recursion depth or increase stack |
| **Kernel says "no memory" but has free RAM** | Kernel virtual space fragmented (need 64-bit kernel) |
| **Driver won't load** | ioremap conflict - check /proc/iomem |

---

### 3. Pages & Page Tables

Memory management operates at "page" granularity (typically 4 KB on ARM):

```
4 KB Page = 4096 bytes (smallest memory unit kernel manages)
Physical page = 4 KB chunk of actual RAM
Virtual page = 4 KB chunk of virtual address space

Page Table = Lookup table mapping virtual pages → physical pages
```

**Page Faults** (when needed page not in memory):

- Minor fault: Page in RAM but not mapped (fast)
- Major fault: Page on disk, needs to load (slow)

---

### 4. Embedded ARM Memory Management

#### 4.1 Virtual vs Physical Address Space

**Definition**:

- **Virtual Address**: Logical address generated by CPU, mapped by MMU to physical location
- **Physical Address**: Actual hardware memory location where data resides

**Mapping Process**:

```
Virtual Address 0x40000000
         ↓ (Page Table Lookup)
Physical Address 0x20200100
         ↓ (MMU Translation)
Actual RAM Chip
```

#### 4.2 ARM 32-bit Address Space Division

BeagleBone Black organizes 4 GB address space into two regions:

**User Space: 0x00000000 - 0x7FFFFFFF (2 GB)**

- Process private memory region
- Each process isolated with independent address mapping
- Contains: TEXT (code), DATA (initialized globals), BSS (uninitialized globals), HEAP, STACK

**Kernel Space: 0x80000000 - 0xFFFFFFFF (2 GB)**

- Shared across all processes
- Contains kernel code, kernel data structures, device I/O mappings
- PAGE_OFFSET constant = 0x80000000

#### 4.3 Physical Memory Organization - Zones

**Problem Statement**:
32-bit kernel virtual space limited to 1 GB (0x80000000 - 0xFFFFFFFF), but physical RAM may exceed addressable limits. Requires zone-based organization for efficient memory management.

**Zone Categories**:

| Zone | Physical Address Range | Virtual Mapping | Characteristics | Use Case | 32-bit? | 64-bit? |
|------|---|---|---|---|---|---|
| ZONE_DMA | 0x00000000 - 0x01000000 (16 MB) | Direct mapping | Legacy ISA DMA constraint | Legacy I/O devices | ✓ Yes | ✓ Yes |
| ZONE_DMA32 | 0x00000000 - 0x100000000 (4 GB) | Direct mapping | Modern hardware DMA | Hardware DMA in first 4GB | ✗ 64-bit only | ✓ Yes |
| ZONE_NORMAL | 0x01000000 - 0x37FFFFFF (16 MB - 896 MB) | 0xC0000000+ Direct 1:1 | Permanently kernel-mapped (LOWMEM) | kmalloc() allocations | ✓ Yes | ✓ Yes |
| ZONE_HIGHMEM | 0x38000000+ (896 MB+) | Temporary 128 MB window | Mapped on-demand via 128 MB region | vmalloc(), large allocs | ✓ 32-bit only | ✗ No |

**BeagleBone Black (512 MB RAM)**:

```
Physical RAM: 0x20000000 - 0x3FFFFFFF

Entire physical RAM falls within ZONE_NORMAL:
└─ Kernel virtual mapping: 0xC0000000 - 0xDFFFFFFF
```

**1 GB System (if available)**:

```
Physical RAM: 0x20000000 - 0x40000000

Split into zones:
├─ ZONE_NORMAL: 0x20000000 - 0x370FFFFF (896 MB)
│  └─ Maps to: 0xC0000000 - 0xDFFFFFFF
│
└─ ZONE_HIGHMEM: 0x37100000 - 0x40000000 (remaining)
   └─ Temporary mapping on-demand via kmap()
```

#### 4.4 Kernel Virtual Space Layout

```
Virtual Address  ┌──────────────────────────┐
0xFFFFFFFF       │ Kernel Modules (insmod)  │ 256 MB
                 │ Loaded driver code       │
0xF0000000       ├──────────────────────────┤
                 │ Device I/O Mappings      │ 256 MB
                 │ (ioremap regions)        │
0xE0000000       ├──────────────────────────┤
                 │ Kernel Heap (kmalloc)    │ 512 MB
                 │ SLAB allocator           │
0xC0000000       ├──────────────────────────┤
                 │ Direct Physical Mapping  │ 1 GB
                 │ ZONE_NORMAL mapped 1:1   │
0x80000000       └──────────────────────────┘
                 PAGE_OFFSET boundary
```

#### 4.5 Physical Address to Virtual Mapping

**Direct Mapping (ZONE_NORMAL)**:

```
Relationship: virt_addr = phys_addr - 0x20000000 + 0xC0000000

Example:
Physical: 0x20000100 → Virtual: 0xC0000100
Physical: 0x30000000 → Virtual: 0xD0000000
```

**Device I/O Mapping (ioremap)**:

```
Physical Hardware: 0x44E00000 (GPIO base)
         ↓ (ioremap mapping)
Virtual Kernel: 0xF0E00000 (allocated by kernel)
         ↓ (accessed by driver)
Driver reads/writes virtual address
```

#### 4.6 Hardware Peripheral Access Pattern

**Kernel Driver Implementation**:

```c
// 1. Map physical hardware address to virtual address
void __iomem *gpio_base = ioremap(0x44E00000, 0x1000);
if (!gpio_base)
    return -ENOMEM;

// 2. Access hardware via virtual address
uint32_t gpio_status = readl(gpio_base + GPIO_DATAIN_OFFSET);
writel(gpio_status | LED_MASK, gpio_base + GPIO_DATAOUT_OFFSET);

// 3. Unmap when finished
iounmap(gpio_base);
```

**ioremap() Operation**:

- Allocates kernel virtual address from Device I/O region (0xF0000000-0xE0000000)
- Creates page table entries mapping virtual → physical
- Returns kernel virtual address for driver use
- Driver never directly accesses physical address

#### 4.7 Address Conversion Functions

**Kernel-space Conversions (ZONE_NORMAL only)**:

```c
#include <asm/page.h>

// Virtual (kernel) → Physical (ZONE_NORMAL)
phys_addr_t phys = virt_to_phys(kernel_vaddr);
// Internally: phys = vaddr - PAGE_OFFSET

// Physical (ZONE_NORMAL) → Virtual (kernel)
void *virt = phys_to_virt(phys_addr);
// Internally: virt = phys + PAGE_OFFSET

// Alternative macros
phys_addr_t phys = __pa(kernel_vaddr);
void *virt = __va(phys_addr);
```

**Limitations**:

- Functions valid only for ZONE_NORMAL addresses
- Invalid for user-space pointers
- Invalid for vmalloc() allocated memory
- Invalid for ZONE_HIGHMEM regions (use kmap_atomic() instead)

#### 4.8 Three-Level Memory Hierarchy

```
Level 1: Kernel Source Code (uses virtual addresses)
         │
         ├─ kmalloc(size, GFP_KERNEL)  → allocates ZONE_NORMAL
         ├─ ioremap(phys, size)        → allocates Device I/O region
         └─ vmalloc(size)              → allocates ZONE_NORMAL + ZONE_HIGHMEM

         ↓ (via Page Tables)

Level 2: Virtual Address Space
         ├─ Kernel Space (0x80000000 - 0xFFFFFFFF)
         │  ├─ 0xC0000000 - 0x80000000: Direct ZONE_NORMAL mapping
         │  ├─ 0xF0000000 - 0xE0000000: Device I/O mappings
         │  └─ 0xFFFFFFFF - 0xF0000000: Kernel modules
         │
         └─ User Space (0x00000000 - 0x7FFFFFFF)
            ├─ TEXT, DATA, BSS, HEAP, STACK
            └─ Per-process private mapping

         ↓ (via MMU Translation)

Level 3: Physical Address Space (Hardware)
         ├─ 0x20000000 - 0x3FFFFFFF: DDR3 RAM (ZONE_NORMAL)
         ├─ 0x44E00000 - 0x44E0FFFF: GPIO Peripheral
         ├─ 0x44E09000 - 0x44E09FFF: UART Peripheral
         └─ Other device registers
```

#### 4.9 Zone Selection for Allocation

**Decision Matrix**:

| Requirement | Zone | Function | Reason |
|---|---|---|---|
| DMA capable | ZONE_NORMAL | kmalloc(GFP_DMA) | Hardware needs physical contiguity |
| Kernel data | ZONE_NORMAL | kmalloc(GFP_KERNEL) | Fast, permanent mapping |
| Interrupt context | ZONE_NORMAL | kmalloc(GFP_ATOMIC) | No sleeping allowed |
| Large buffer (>4 MB) | ZONE_NORMAL or HIGHMEM | vmalloc() | Size exceeds kmalloc limit |
| Temporary access | ZONE_HIGHMEM | kmap_atomic() | Temporary kernel mapping |

#### 4.10 Complete Memory Access Example

**Scenario**: Reading GPIO input on BeagleBone

```c
#define GPIO0_PHYS_BASE    0x44E00000
#define GPIO_DATAIN_OFFSET 0x38
#define GPIO_DATAOUT_OFFSET 0x3C

// Kernel module code
static void __iomem *gpio_virt_base;

int init_gpio_driver(void) {
    // Step 1: Map physical GPIO to kernel virtual address
    gpio_virt_base = ioremap(GPIO0_PHYS_BASE, 4096);
    if (!gpio_virt_base) {
        printk(KERN_ERR "ioremap failed\n");
        return -ENOMEM;
    }
    
    // gpio_virt_base now contains virtual address (e.g., 0xF0E00000)
    // This is in Device I/O region
    return 0;
}

uint32_t read_gpio_input(void) {
    // Step 2: Read GPIO input register via virtual address
    // Virtual: gpio_virt_base + GPIO_DATAIN_OFFSET
    // MMU translates: 0xF0E00000 + 0x38 → 0x44E00000 + 0x38
    // Hardware receives access to physical GPIO register
    return readl(gpio_virt_base + GPIO_DATAIN_OFFSET);
}

void write_gpio_output(uint32_t value) {
    // Step 3: Write GPIO output register via virtual address
    writel(value, gpio_virt_base + GPIO_DATAOUT_OFFSET);
}

void cleanup_gpio_driver(void) {
    // Step 4: Unmap kernel virtual address
    iounmap(gpio_virt_base);
    gpio_virt_base = NULL;
}
```

**Address Resolution**:

```
Driver code: readl(0xF0E00038)
         ↓ (Page Table Lookup)
Physical: readl(0x44E00038)
         ↓ (GPIO Peripheral)
Hardware: GPIO register reads input state
```

---

### 5. Why Physical Address Space Matters for Embedded

Device drivers must:

1. Know peripheral physical addresses (from SoC datasheet)
2. Map physical → virtual using `ioremap()`
3. Access via virtual addresses using `readl()` / `writel()`
4. Cleanup with `iounmap()`

Example: BeagleBone PWM driver needs to know:

- TIMER physical address: 0x4A30C000
- TIMER interrupt: 68
- Register offsets for PWM configuration
- How to enable module clocks (PRCM register at 0x44C0)

**The Bottom Line**: Understanding physical addresses is essential for embedded driver development on ARM boards.

---

```
0xFFFFFFFF ┌────────────────────┐
           │ KERNEL SPACE (1GB) │ 0xC0000000 - 0xFFFFFFFF
0xC0000000 ├────────────────────┤ PAGE_OFFSET
           │ USER SPACE (3GB)   │ 0x00000000 - 0xBFFFFFFF
0x00000000 └────────────────────┘

x86: PAGE_OFFSET = 0xC0000000
ARM: PAGE_OFFSET = 0x80000000
```

**Why Kernel in Every Process?**

- Every process uses system calls → kernel code needed
- Avoids expensive address space switching on kernel entry/exit

---

### 3. Pages & Memory Management

**What is a Page?**

- Fixed-size contiguous block of memory (typically 4096 bytes = 4 KB)
- Used as basic unit for memory management

**Key Terms**:

- **Virtual Page**: Kernel represents with `struct page`
- **Page Frame**: Physical memory block (has Page Frame Number - PFN)
- **Page Table**: Maps virtual pages to physical page frames

**Find Page Size**:

```bash
getconf PAGESIZE    # Returns 4096 (bytes)
```

**struct page** (simplified) - Kernel represents every physical page:

```c
#include <linux/mmtypes.h>

struct page {
    unsigned long flags;    // Page status (Dirty, locked, etc.) - see <linux/page-flags.h>
    atomic_t _count;        // Reference count: -1 when free, >0 when in use
    void *virtual;          // Virtual address (ZONE_NORMAL) or NULL (ZONE_HIGHMEM)
    // ... additional fields
};

// Helper macros for page/frame conversion:
page_to_pfn(page);          // struct page → Page Frame Number
pfn_to_page(pfn);           // Page Frame Number → struct page
```

**Find Page Size** (in bytes):

```bash
getconf PAGESIZE             # Returns 4096 (most common)
getconf PAGE_SIZE            # Alternative command
grep -i pagesize /proc/self/status  # Also shows page size
```

**struct page Overhead Analysis**:

Every physical page (4 KB) has a corresponding struct page (typically 64 bytes on 32-bit ARM):

```
Physical Memory: 4 GB system
  Total pages: 4 GB ÷ 4 KB = 1,048,576 pages
  struct page overhead: 1,048,576 × 64 bytes = 64 MB (for metadata!)
  
This is why kernel needs to track all pages efficiently
```

---

### 4. Kernel Memory: Eager vs User Memory: Lazy

**Kernel Memory** (Eager Allocation):

- NOT demand-paged (no lazy allocation)
- Every kmalloc() gets REAL physical memory immediately
- Never paged out or swapped
- More predictable, lower latency

**User Space** (Lazy Allocation):

- malloc() returns virtual address WITHOUT physical RAM
- Page table entries marked with flag to trap access
- First page access → **Page Fault** exception

**Page Faults - 2 Types** (Major vs Minor):

1. **Minor Fault** (Fast):
   - Page already in RAM but not mapped for process
   - Kernel finds page in memory → maps it → returns
   - Example: First access to malloc'd memory, page already in RAM
   - Latency: ~μs (microseconds)

2. **Major Fault** (Slow):
   - Page not in RAM, mapped to file (e.g., via mmap)
   - Kernel must read page from disk → IO operation → map it → return
   - Example: Accessing mmap'd large file, page not cached in memory
   - Latency: ~ms (milliseconds) - 1000x slower than minor fault

**Why the difference?**

```
Minor Fault:     RAM lookup → map → done (fast)
Major Fault:     Disk I/O → RAM read → map → done (slow)
```

**Check Page Faults** (in user space):

```bash
# Per-process page fault statistics (C API)
#include <sys/resource.h>
struct rusage usage;
getrusage(RUSAGE_SELF, &usage);
printf("Major faults: %ld, Minor faults: %ld\n", usage.ru_majflt, usage.ru_minflt);

# Or via /proc
cat /proc/[pid]/stat | awk '{print "Majflt: " $12 ", Minflt: " $10}'
```

---

### 5. User Space Memory Layout

**Process Memory** (low to high address):

```
High ┌──────────────────────────┐
     │ Arguments & Environment  │
     ├──────────────────────────┤
     │ STACK (grows down)       │ Local vars, return addresses
     │         ↓↓↓              │
     ├──────────────────────────┤
     │ [UNMAPPED REGION]        │
     │         (hole)           │
     ├──────────────────────────┤
     │ HEAP (grows up)          │ malloc/free area
     │         ↑↑↑              │
     ├──────────────────────────┤
     │ BSS (uninitialized data) │ Static vars (zeroed)
     ├──────────────────────────┤
     │ Initialized Data         │ Global vars (from file)
     ├──────────────────────────┤
     │ TEXT (code)              │ Executable instructions
Low  └──────────────────────────┘
```

**View Process Memory**:

```bash
cat /proc/[pid]/maps  # Shows detailed memory layout
```

---

### 6. Low Memory & High Memory Zones

**Problem in 32-bit Systems**:

- Kernel virtual space = only 1 GB (3GB-4GB)
- Physical RAM can exceed 1 GB
- Need different handling for different RAM ranges

**Solution - Zones**:

```
32-bit Kernel Virtual Space (1 GB):
4 GB    ┌──────────────────────────┐
        │ HIGHMEM (128 MB)         │ Temporary mappings for
3.87 GB │ Temp mapping region      │ physical RAM > 896 MB
        ├──────────────────────────┤
        │ LOWMEM (896 MB)          │ Permanent mappings for
3 GB    │ Direct access            │ first 896 MB of RAM
        └──────────────────────────┘
```

**Physical RAM Zones**:

- **ZONE_DMA**: 0-16 MB (legacy hardware)
- **ZONE_NORMAL**: 16-896 MB (standard allocations)
- **ZONE_HIGHMEM**: 896+ MB (32-bit only, temporary access)

---

### 7. Low Memory Details

**LOWMEM** (first 896 MB):

- Permanently mapped by kernel at boot
- Virtual addresses = **Logical Addresses** (permanent, fixed offset)
- Simple conversion (subtract fixed offset)

**Address Conversion**:

```c
#include <asm/page.h>

phys_addr_t phys = virt_to_phys(kernel_virt);  // Virtual → Physical
void *virt = phys_to_virt(phys);               // Physical → Virtual

// Or use:
phys_addr_t phys = __pa(kernel_virt);
void *virt = __va(phys);
```

**Zone Example** (512 MB system):

- ZONE_DMA: 0-16 MB (16 MB)
- ZONE_NORMAL: 16-512 MB (496 MB)
- ZONE_HIGHMEM: none

---

### 8. High Memory Details

**HIGHMEM** (physical RAM > 896 MB):

- Cannot be permanently mapped (kernel virtual space too small)
- Kernel creates temporary mappings on-demand
- Pattern: map → use → unmap

**Performance**: Slower than LOWMEM (requires mapping/unmapping)

**64-bit Systems**: No high memory concept needed (2^64 address space huge)

- **Purpose**: Top 128 MB of kernel address space for temporary mapping of physical memory > 1GB
- **Access Method**: Kernel creates temporary mappings on-demand
- **Performance**: High memory access slower than low memory
- **Mapping Lifecycle**: Created on-the-fly, destroyed when done
- **64-bit Systems**: No high memory concept (2^64 huge address range eliminates need)

---

### 9. Memory Allocation Hierarchy

**Kernel Memory Allocators** - Two-Tier Layered Architecture:

```
┌──────────────────────────────────────────────────┐
│     Driver/Kernel Code (allocates memory)        │
└──────────────────────────────────────────────────┘
         ↓                ↓                    ↓
    kmalloc()        vmalloc()         get_free_pages()
         ↓                ↓                    ↓
    ┌──────────┐         │              ┌──────────────┐
    │ SLAB     │         │              │   Page       │
    │ Caches   │         │              │ Allocator    │
    │ (small   │         │              │ (Buddy-based)│
    │ objects) │         │              │              │
    └────┬─────┘         │              └────┬─────────┘
         │               │                   │
         └───────────────┼───────────────────┘
                         ↓
        ┌────────────────────────────────────┐
        │  Page Allocator (Buddy System)     │
        │  Manages 4 KB physical pages       │
        │  in powers of 2 (4KB, 8KB, 16KB)   │
        └────────────────┬───────────────────┘
                         ↓
    Buddy Allocator Structure:
    
    ┌─────────────────────────────────────────────────────┐
    │     Free Pages by Order (Buddy System)              │
    ├─────────────────────────────────────────────────────┤
    │ Order 0:  [●●●●●●●●●●]  (1 page = 4 KB)             │
    │ Order 1:  [◆◆◆◆◆]    (2 pages = 8 KB)             │
    │ Order 2:  [■■■]         (4 pages = 16 KB)           │
    │ Order 3:  [◈◈]         (8 pages = 32 KB)           │
    │ Order 4:  [▓]           (16 pages = 64 KB)          │
    │ Order 5:  [ ]           (32 pages = 128 KB)         │
    │ Order 6:  [ ]           (64 pages = 256 KB)         │
    │ Order 7:  [ ]           (128 pages = 512 KB)        │
    │ Order 8:  [ ]           (256 pages = 1 MB)          |
    │ Order 9:  [ ]           (512 pages = 2 MB)          │
    │ Order 10: [ ]           (1024 pages = 4 MB)         |
    │                                                     │
    │ Coalescing: 2 free buddies → merge to next order    │
    │ Splitting: Order N → 2 × Order (N-1)                │
    └─────────────────────────────────────────────────────┘
                         ↓
                 ┌─────────────────┐
                 │  Physical RAM   │
                 │ (Managed by MMU)│
                 └─────────────────┘
```

**Two-Layer Allocator System**:

| Allocator | Level | What It Does | Manages | Speed |
|-----------|-------|---|---|---|
| **SLAB** | TOP (Frontend) | Carves pages into small fixed-size objects (cache layer) | Small chunks (32B-32KB) | Ultra-Fast (~1μs cached) |
| **Page Allocator** | BOTTOM (Backend) | Manages physical pages using Buddy algorithm, coalesces/splits in powers of 2 | Individual/blocks of 4 KB pages | Fast (~100-500 ns) |

**How They Work Together**:

| Function | Route | Zone (32-bit) | Size | Purpose | Path |
|----------|-------|---|------|---------|---|
| **kmalloc()** | → SLAB → Page Allocator | ZONE_NORMAL | ≤4 MB | Fast small allocations, pre-cached | 2 layers |
| **vmalloc()** | → Page Allocator (direct) | ZONE_HIGHMEM | Large | Virtual mapping, scattered pages, temporary | 1 layer |
| **get_free_pages()** | → Page Allocator (direct) | ZONE_NORMAL | Pages (4KB+) | Direct page allocation, physically contiguous | 1 layer |

**Why Two Layers?**

- **SLAB** (top): Pre-caches frequently used sizes from ZONE_NORMAL → ultra-fast allocation (~1μs)
- **Page Allocator** (bottom): Uses Buddy System to efficiently manage physical pages in any zone → any size allocation

**Detailed Allocator Comparison**:

| Aspect | Page Allocator (Buddy-based) | SLAB Allocator |
|--------|---|---|
| **What it does** | Manages physical pages in powers of 2 (4KB, 8KB, 16KB...) | Carves ZONE_NORMAL pages into small fixed-size objects |
| **Allocates** | Page blocks (4 KB, 8 KB, 16 KB, 32 KB...) | Small chunks (32B, 64B, 128B, 256B...) |
| **Used by** | `vmalloc()` (ZONE_HIGHMEM), `get_free_pages()` + SLAB backend | `kmalloc()` frontend only (ZONE_NORMAL) |
| **Zone** | ZONE_HIGHMEM (vmalloc), ZONE_NORMAL (other) | ZONE_NORMAL only |
| **Speed** | Fast (~100-500 ns) | Ultra-fast (~1 μs cached) or ~2 μs (cache miss) |
| **Fragmentation** | Low (powers of 2 coalescing) | Very low (cache reuse, no fragmentation) |
| **Overhead** | Low | Tiny per-object |
| **Best for** | Direct page allocation, scattered mapping (vmalloc) | Small frequent allocations (kmalloc) |
| **Algorithm** | Buddy System: coalesce/split free blocks | Object cache with pre-carved sizes |

---

**Data Flow Example 1** (kmalloc 256 bytes - SLAB + Page):

```
Driver calls: ptr = kmalloc(256, GFP_KERNEL)
       ↓
SLAB Allocator (Top layer):
   "Check 256-byte cache..."
       ↓
   If cache HIT → Return block (FAST ~1μs)
       ↓ (if cache MISS)
   "Need a page from Page Allocator"
       ↓
Page Allocator (Bottom layer - Buddy-based):
   "Find 4 KB block via Buddy algorithm"
       ↓
   If 4 KB block available → Use it
       ↓ (if not available)
   Coalesce free blocks or allocate from physical RAM
       ↓
Returns 4 KB page to SLAB
       ↓
SLAB carves 4 KB into 16× 256-byte blocks
SLAB caches 15 blocks for reuse
Returns 1 block to driver
```

**Data Flow Example 2** (vmalloc 1 MB - Page Allocator direct):

```
Driver calls: ptr = vmalloc(1024*1024)
       ↓
SLAB: SKIPPED (vmalloc bypasses SLAB)
       ↓
Page Allocator (Bottom layer - Buddy-based):
   "Need 256 pages (4 KB each)"
       ↓
Buddy System:
   Allocates 256 scattered physical pages
   (not necessarily contiguous)
       ↓
Kernel creates virtual address mappings
(Makes scattered pages appear contiguous virtually)
       ↓
Returns: Single virtual address to driver
(Physically scattered but virtually contiguous)
```

**Data Flow Example 3** (get_free_pages 8 KB - Page Allocator direct):

```
Driver calls: ptr = get_free_pages(GFP_KERNEL, 1)  // order=1 → 2 pages
       ↓
SLAB: SKIPPED
       ↓
Page Allocator (Bottom layer - Buddy-based):
   "Allocate 2 contiguous pages"
       ↓
Buddy System:
   Finds or coalesces 2 pages (8 KB)
   Marks pages as "in use"
   Returns physical page address
       ↓
Returns: Physical page address to driver
(Direct control, no caching via SLAB)
```

---

**Decision Matrix - Which Allocator to Use**:

| Requirement | Function | Route | Why |
|---|---|---|---|
| Small (< 4 KB), fast | `kmalloc()` | SLAB→Page | Pre-cached, ~1μs |
| Medium (4 KB - 4 MB) | `kmalloc()` | SLAB→Page | Still cached |
| Large (> 4 MB) | `vmalloc()` | Page (direct) | Scattered OK, no size limit |
| Page-aligned buffer | `get_free_pages()` | Page (direct) | Direct Buddy control, no overhead |
| DMA operations | `kmalloc(GFP_DMA)` | SLAB→Page | Must be physically contiguous |
| Interrupt context | `kmalloc(GFP_ATOMIC)` | SLAB→Page | Cannot sleep, no vmalloc() |

---

**Deep Dive: The Two-Tier Allocator System**

**Layer 1: Page Allocator with Buddy System (Bottom - Core Layer)**

The Page Allocator is the foundation using the **Buddy Algorithm** to manage physical pages:

```
Page Allocator Responsibilities:
├─ Manages which 4 KB pages are free vs in-use
├─ Uses Buddy System for efficient coalescing/splitting
├─ Combines pages into blocks of powers of 2 (4KB, 8KB, 16KB...)
├─ Splits larger blocks when smaller allocations needed
└─ Direct interface to physical RAM via MMU
```

**Page Allocator Characteristics**:

- Uses **Buddy System algorithm** (internal - not separate layer)
- Allocates powers of 2: 4KB, 8KB, 16KB, 32KB, 64KB, 128KB, 256KB, 512KB, 1MB
- Very efficient: coalesces free "buddy" blocks for low fragmentation
- Backend for both vmalloc() and SLAB allocator
- Speed: ~100-500 nanoseconds per allocation

**Typical Usage**:

```c
unsigned long page = __get_free_page(GFP_KERNEL);       // Get 1 page (4 KB)
struct page *pages = alloc_pages(GFP_KERNEL, 2);        // Get 4 pages (2^2 = 4)
void *virt = vmalloc(1024*1024);                        // Allocate 1 MB via Buddy
```

---

**Buddy Algorithm: Internal Mechanism of Page Allocator**

The Buddy System is the algorithm used BY the Page Allocator (not a separate layer):

```
Buddy System Responsibilities:
├─ Manages free blocks of pages in powers of 2
├─ Combines adjacent "buddy" blocks when both are free
├─ Splits larger blocks when smaller allocations needed
├─ Minimizes external fragmentation through efficient coalescing
└─ Powers of 2: 4KB, 8KB, 16KB, 32KB, 64KB, 128KB, 256KB, 512KB, 1MB...
```

**Buddy Algorithm Example**:

```
Request: 8 KB allocation

State: Free blocks = [4KB] [4KB] [4KB] [4KB]
       ↓
Buddy System finds: No 8 KB block available
       ↓
Coalesces two "buddy" 4 KB blocks: [4KB]+[4KB] → [8KB]
       ↓
Returns 8 KB to requester
       ↓
Result: Two adjacent 4 KB blocks now "paired" as buddies
```

**Why "Buddy"?** Pages are paired (buddies). When both are free, they automatically combine into a larger block. When a large block is split, the resulting two halves become buddies. This ensures efficient memory management with low fragmentation.

**Buddy System Characteristics**:

- Always allocates and combines in powers of 2
- Very efficient at coalescing/splitting
- Low external fragmentation compared to arbitrary-size allocators
- Operates inside Page Allocator (transparent to users of kmalloc/vmalloc)

---

**Layer 2: SLAB Allocator (Top - Caching Layer)**

The SLAB allocator sits on top of the Page Allocator and provides pre-cached small objects:

```
SLAB Allocator Responsibilities:
├─ Requests 4 KB pages from Page Allocator
├─ Carves pages into fixed-size chunks (32B, 64B, 128B...)
├─ Caches commonly used sizes for ultra-fast reuse
├─ Returns pre-allocated blocks from cache
└─ Eliminates fragmentation for small allocations
```

**SLAB Caching Strategy**:

```
Memory Request: kmalloc(100 bytes)
       ↓
SLAB checks: "Do I have a 128-byte cache?"
       ↓
Cache HIT (exists and has free blocks):
   Return pre-cut block immediately (FAST ~1μs)
       ↓
Cache MISS (empty or doesn't exist):
   Request 4 KB page from Page Allocator
   Carve into 32× 128-byte blocks
   Cache 31 blocks for future reuse
   Return 1 block to requester (SLOWER ~2μs first time)
```

**SLAB Cache Sizes** (ARM 32-bit):
32B, 64B, 128B, 256B, 512B, 1KB, 2KB, 4KB, 8KB, 16KB, 32KB

**Example - Request 100 bytes**:

```
Allocation: 100 bytes requested
       ↓
SLAB rounds UP to next power of 2: 128 bytes
       ↓
Returns: 128 bytes allocated (overhead = 28 bytes)

Query with ksize(ptr): Returns 128
```

**SLAB Advantages**:

- Pre-cached → ultra-fast (1 μs)
- Low fragmentation (objects perfectly aligned)
- Kernel can track object types easily
- Memory locality good (cache-friendly)

**SLAB Disadvantages**:

- Limited to 4 MB max (Page Allocator backend limit)
- Fixed sizes only (wastes space for odd sizes)
- Cannot use in interrupt context for allocation (depends on GFP flags)

---

**Buddy Allocator Coalescing Behavior**:

```
Scenario: Multiple 4 KB pages becoming free

Initial state (busy):  [BUSY] [BUSY] [BUSY] [BUSY]
                        4 KB   4 KB   4 KB   4 KB

Free one pair:         [FREE] [FREE] [BUSY] [BUSY]
       ↓
Buddy coalesces:       [8 KB FREE] [BUSY] [BUSY]
                        (paired buddies merge)

Free another pair:     [8 KB FREE] [FREE] [FREE]
       ↓
Coalesce again:        [8 KB FREE] [8 KB FREE]
       ↓
Coalesce to 16 KB:     [16 KB FREE]
```

---

**When Each Layer Is Invoked**:

| Driver Code | Allocation Path | Layers Used | Typical Time |
|---|---|---|---|
| `ptr = kmalloc(256, GFP_KERNEL)` | SLAB→Page | 2 layers | ~1μs (cache hit) |
| `ptr = kmalloc(256, GFP_KERNEL)` first time | SLAB→Page | 2 layers | ~2μs (cache miss) |
| `ptr = vmalloc(1MB)` | Page (direct) | 1 layer | ~100-500 ns (Buddy) |
| `page = get_free_pages(GFP_KERNEL, 0)` | Page (direct) | 1 layer | ~100-500 ns (Buddy) |

---

**Deep Dive: How SLAB Allocator Works**

The SLAB allocator sits between kmalloc and Buddy allocator:

```
Memory Request: kmalloc(100 bytes)
       ↓
SLAB Allocator checks: "Do I have a 128-byte cache?"
       ↓
   Cache Hit? → Return pre-allocated block (FAST ~1μs)
       ↓ (if miss)
   Request 4 KB page from Buddy allocator
       ↓
   Carve 4 KB into 32× (128-byte blocks)
       ↓
   Fill cache with these blocks
       ↓
   Return one block to caller
```

**SLAB Cache Sizes** (ARM 32-bit):
32B, 64B, 128B, 256B, 512B, 1KB, 2KB, 4KB, 8KB, 16KB, 32KB

**Example**: Request 100 bytes → SLAB gives 128 bytes (overhead = 28 bytes)

**Buddy Allocator Behavior**:

```
Free blocks: [4KB] [4KB] [4KB] [4KB]
Request: 8 KB
   ↓
Coalesce two 4 KB blocks → [8 KB]
   ↓
Return 8 KB to caller
```

---

### 10. kmalloc() Function

**Header**:

```c
#include <linux/slab.h>
```

**Prototype**:

```c
void *kmalloc(size_t size, gfp_t flags);
```

**Characteristics**:

- ✅ Memory contiguous in virtual AND physical space
- ✅ Suitable for hardware DMA/PCI
- ✅ Lower overhead
- ❌ Max 4 MB per allocation
- ❌ Can cause fragmentation

**Maximum Size**: 4 MB (2^22 bytes)

```
Calculation:
KMALLOC_MAX_SIZE = 2^(MAX_ORDER + PAGE_SHIFT - 1)
                 = 2^(11 + 12 - 1)
                 = 2^22 = 4 MB
```

**GFP Flags**:

| Flag | Context | Zone | Use | Sleeps? |
|------|---------|------|-----|----------|
| GFP_KERNEL | Normal code | ZONE_NORMAL (LOWMEM) | Default, blocks/schedules | ✓ Yes |
| GFP_ATOMIC | Interrupt, spinlock | ZONE_NORMAL (LOWMEM) | No sleep, no blocking | ✗ No |
| GFP_DMA | Any | ZONE_DMA (0-16MB) | Legacy hardware only | Depends |
| GFP_DMA32 | 64-bit systems | ZONE_DMA32 (0-4GB) | Modern hardware DMA | Depends |
| GFP_HIGHUSER | User context (32-bit) | ZONE_HIGHMEM | User space buffers | ✓ Yes |
| GFP_USER | User context | ZONE_NORMAL | User memory allocation | ✓ Yes |

**Usage Example**:

```c
void *ptr = kmalloc(1024, GFP_KERNEL);
if (!ptr)
    return -ENOMEM;
// Use ptr...
kfree(ptr);
```

**Special Case: kmalloc(0)**:

```c
void *ptr = kmalloc(0, GFP_KERNEL);  // Allocates ZERO_SIZE_PTR (non-NULL)
if (ptr == NULL)                     // NULL check may FAIL!
    return -ENOMEM;                  // Won't execute
// ptr is a special non-dereferenceable value
*ptr = 1;                            // FAULT: crashes
kfree(ptr);                          // OK: handles ZERO_SIZE_PTR correctly
```

**Note**: kmalloc(0) returns a special ZERO_SIZE_PTR value that looks like a valid pointer but crashes if dereferenced. kfree() is safe with this value.

**kfree() - Always Match kmalloc**:

```c
void kfree(const void *ptr);  // Safe with NULL and ZERO_SIZE_PTR
```

Rules:

- Call exactly once per kmalloc()
- Never double-free
- Only with kmalloc() pointers (never use with vmalloc, stack, or user memory)

---

## COMPREHENSIVE MEMORY ALLOCATION FUNCTIONS

### A. kmalloc() - Kernel Memory Allocation

```c
#include <linux/slab.h>

void *kmalloc(size_t size, gfp_t flags);  // Returns NULL on failure
void kfree(const void *ptr);              // Safe with NULL
```

**Key Characteristics**: Physical + virtual contiguous, max 4 MB, DMA safe, SLAB rounded allocation

**GFP Flags**:

| Flag | When | Context |
|------|------|---------|
| `GFP_KERNEL` | Normal code, can sleep | Default choice |
| `GFP_ATOMIC` | Interrupts, spinlocks | Cannot sleep |
| `GFP_DMA` | Legacy hardware | Low memory (0-16 MB) |

**Typical Usage**:

```c
struct device_config *cfg = kmalloc(sizeof(*cfg), GFP_KERNEL);
if (!cfg) return -ENOMEM;
cfg->id = 42;
kfree(cfg);
```

**SLAB Rounding**: Allocating 100 bytes actually allocates 128 bytes (power of 2)

**Limit**: Cannot allocate > 4 MB. Use vmalloc() for larger sizes.

---

### B. vmalloc() - Virtual Memory Allocation (HIGHMEM on 32-bit)

```c
#include <linux/vmalloc.h>

void *vmalloc(unsigned long size);     // Virtual contiguous, physically scattered (HIGHMEM on 32-bit)
void *vzalloc(unsigned long size);     // vmalloc + zero initialization
void vfree(const void *ptr);           // Always use vfree() for vmalloc (NOT kfree!)
```

**Key Characteristics**:

- Virtual contiguous but physical scattered (non-contiguous)
- Allocates from ZONE_HIGHMEM on 32-bit systems (via 128MB temporary mapping)
- No size limit (limited by available RAM and kernel virtual space)
- NOT DMA safe (hardware can't follow scattered pages)
- Slower than kmalloc (requires page table setup, TLB invalidation)
- Cannot use in interrupt context (requires page table allocation)

**Difference from kmalloc**:

```
kmalloc: Physical pages CONTIGUOUS → Fast, DMA-safe, ≤4MB, in ZONE_NORMAL
vmalloc: Physical pages SCATTERED → Slower, not DMA, unlimited, in ZONE_HIGHMEM (32-bit)
```

**Usage**:

```c
void *large_buf = vmalloc(10 * 1024 * 1024);  // 10 MB allocation
if (!large_buf) return -ENOMEM;
memset(large_buf, 0, 10 * 1024 * 1024);
vfree(large_buf);  // CRITICAL: Use vfree(), NOT kfree()!
```

**Cannot use with DMA**: vmalloc'd memory is physically scattered; hardware DMA needs contiguous. Use kmalloc(GFP_DMA) or kmalloc(GFP_DMA32) instead.

---

### C. malloc() - User-Space Memory Allocation

```c
#include <stdlib.h>

void *malloc(size_t size);
void *calloc(size_t count, size_t size);  // malloc + zero initialization
void *realloc(void *ptr, size_t size);    // Resize existing allocation
```

**Key Differences from kmalloc**:

| | malloc | kmalloc |
|---|--------|---------|
| Used by | User-space apps | Kernel |
| Allocation | Lazy (page fault on access) | Immediate (eager) |
| Persistence | Freed on exit | Leaked if not freed |
| Max size | ~2 GB per process | 4 MB |
| Paging | Can be paged to swap | Never paged |

**Typical usage**: `int *arr = malloc(100 * sizeof(int)); free(arr);`

---

### D. kzalloc() - Zero-Initialized Kernel Allocation

```c
#include <linux/slab.h>

void *kzalloc(size_t size, gfp_t flags);  // = kmalloc() + memset(0)
```

**Benefit**: Single call instead of kmalloc + memset

```c
struct config *cfg = kzalloc(sizeof(*cfg), GFP_KERNEL);  // Already zeroed
if (!cfg) return -ENOMEM;
cfg->version = 1;  // All other fields are 0
kfree(cfg);
```

---

### E. krealloc() - Resize Kernel Memory

```c
#include <linux/slab.h>

void *krealloc(const void *ptr, size_t new_size, gfp_t flags);
```

**Behavior**: Allocates new block, copies data, frees old block

```c
int *arr = kmalloc(10 * sizeof(int), GFP_KERNEL);
// Later: need 20 integers
arr = krealloc(arr, 20 * sizeof(int), GFP_KERNEL);
if (!arr) return -ENOMEM;  // Old block still valid if realloc fails
```

---

### F. ksize() - Query Actual Allocated Size

```c
#include <linux/slab.h>

size_t ksize(const void *ptr);  // Returns actual allocated bytes
```

**Why**: SLAB rounds up to power of 2 (request 100 bytes → allocate 128 bytes)

```c
void *ptr = kmalloc(100, GFP_KERNEL);
printk("Requested: 100, Actual: %zu\n", ksize(ptr));  // Output: 128
```

**Note**: Only works with kmalloc'd pointers, not vmalloc or stack variables.

---

### G. Comprehensive Comparison Matrix

| Function | Type | Max Size | Physical Contiguous | DMA Safe | Interrupt Safe | Sleeps | Returns Zeroed | Use Case |
|----------|------|----------|--|--|--|--|--|--|
| **kmalloc** | Kernel | 4 MB | ✓ Yes | ✓ Yes | ✓ Yes (atomic) | ✗ No | ✗ No | Device drivers, hardware |
| **vmalloc** | Kernel | RAM size | ✗ No | ✗ No | ✗ No | ✓ Yes | ✗ No | Large buffers, video |
| **kzalloc** | Kernel | 4 MB | ✓ Yes | ✓ Yes | ✓ Yes (atomic) | ✗ No | ✓ Yes | Zeroed structures |
| **krealloc** | Kernel | 4 MB | ✓ Yes | ✓ Yes | ✗ No | ✓ Yes | ✗ No | Growing buffers |
| **malloc** | User | 2 GB | ~ Maybe | N/A | N/A | ✓ Yes | ✗ No | User apps |
| **calloc** | User | 2 GB | ~ Maybe | N/A | N/A | ✓ Yes | ✓ Yes | User arrays |
| **realloc** | User | 2 GB | ~ Maybe | N/A | N/A | ✓ Yes | ✗ No | Growing user buffers |

---

### H. Memory Allocation Decision Tree

```
Need memory in kernel?
    ↓
[Size > 4 MB?]
    ├─ YES → vmalloc()          (Large buffers)
    └─ NO
        ↓
    [Need zeros?]
        ├─ YES → kzalloc()      (Structures with many fields)
        └─ NO
            ↓
        [In interrupt?]
            ├─ YES → kmalloc(GFP_ATOMIC)
            └─ NO → kmalloc(GFP_KERNEL)  (Default)
```

---

## ADVANCED ALLOCATION & BUDDY SYSTEM

### 2. Virtual to Physical Address Conversion

**Problem**: CPU uses virtual addresses, hardware needs physical addresses

**Kernel Functions**:

```c
#include <asm/io.h>

// Virtual (kernel) → Physical (ZONE_NORMAL only!)
phys_addr_t phy = virt_to_phys(virt_addr);

// Physical → Virtual (kernel) (ZONE_NORMAL only!)
void *vir = phys_to_virt(phys_addr);

// Alternate macros
phys_addr_t phy = __pa(kernel_vaddr);     // kernel virtual → physical
void *virt = __va(phys_addr);             // physical → kernel virtual
```

**CRITICAL Limitations** (ZONE_NORMAL only):

- ✓ Valid: kmalloc() from ZONE_NORMAL (LOWMEM) memory
- ✗ Invalid: User-space pointers (no kernel mapping exists)
- ✗ Invalid: vmalloc() allocated memory (physically scattered)
- ✗ Invalid: ZONE_HIGHMEM regions (use kmap_atomic() for temporary access)
- ✗ Invalid: Stack variables, kernel structures

**Why these restrictions?**

- virt_to_phys only works for permanently mapped kernel memory
- vmalloc creates temporary mappings that vary at runtime
- User space has per-process mappings (same virtual address = different physical address)
- ZONE_HIGHMEM pages are mapped on-demand and may not have permanent kernel addresses

**Example - Correct Usage** (kmalloc from ZONE_NORMAL):

```c
void *kmem = kmalloc(1024, GFP_KERNEL);  // Allocates from ZONE_NORMAL
phys_addr_t phys = virt_to_phys(kmem);   // CORRECT - works
kfree(kmem);
```

**Example - Incorrect Usage** (vmalloc - will return invalid address):

```c
void *vmem = vmalloc(10 * 1024 * 1024);  // Allocates from ZONE_HIGHMEM
phys_addr_t phys = virt_to_phys(vmem);   // WRONG - undefined/incorrect behavior
vfree(vmem);
```

---

### 3. kmalloc Maximum Size

**Limit**: 4 MB (2^22 bytes)

**Why**: `MAX_ORDER = 11 → 2^(11+12-1) = 2^22 = 4 MB`

---

### 4. Buddy Allocator

The kernel's buddy allocator manages free pages in powers of 2:

```
Free blocks: 1 MB (2^20 bytes)
Allocation: 256 KB requested
Result: Allocates 512 KB (next power of 2)
Remaining: Split larger blocks until finding exact fit
```

**View buddy allocator state**:

```bash
cat /proc/buddyinfo  # Shows free pages by order
```

---

## PRACTICAL DEBUGGING & MEMORY PROFILING

### Check Current Memory Usage

```bash
# System-wide memory
cat /proc/meminfo

# Process-specific memory
cat /proc/[pid]/status

# Show memory map
cat /proc/[pid]/maps
```

### Kernel Memory Leak Detection

```bash
# Enable debug (in driver code)
#define DEBUG 1
#include <linux/slab.h>

// With debug, kmemleak can track allocations
dmesg | grep kmemleak

# Check for unreferenced objects
cat /sys/kernel/debug/kmemleak
```

---

**Solution**: Use vmalloc() for larger allocations

---

### 4. Kernel Memory Leaks

**Question**: What if we don't call kfree()?

**Answer**: Kernel memory is NEVER freed automatically

**Consequences**:

- Memory remains allocated forever (until reboot)
- System gradually exhausts free memory
- Eventually: crash, OOM killer activation

**Prevention**: Always match kmalloc() with kfree()

---

### 5. ksize() - Actual Allocated Size

**Problem**: kmalloc() may allocate more than requested due to SLAB rounding

**Function**:

```c
size_t ksize(const void *ptr);  // Returns actual allocated size
```

**Example**:

```c
void *ptr = kmalloc(100, GFP_KERNEL);
size_t actual = ksize(ptr);  // Maybe 128, 256, etc.
```

**Use**: Detect SLAB rounding, use extra space for additional data

---

### 6. kzalloc() - Zero-Initialized

**Function**:

```c
void *kzalloc(size_t size, gfp_t flags);  // Like kmalloc() + memset(0)
```

**Benefit**: No separate memset() call needed

---

### 7. krealloc() - Resize Memory

**Function**:

```c
void *krealloc(const void *p, size_t new_size, gfp_t flags);
```

**Like**: realloc() in user space (allocate new block, copy old data)

---

### 8. Physical Contiguity Benefits

**Why kmalloc() Guarantees Physical Contiguity**:

1. Hardware DMA controllers can't follow page tables
2. Single large page mapping = fewer TLB misses
3. Better performance and predictability

**Trade-off**: Hard to find large contiguous blocks after system uptime

---

### 9. vmalloc() - Virtual Contiguity

**Key Difference**: Virtually contiguous BUT NOT physically contiguous

**Characteristics**:

- Pages can be scattered anywhere in physical RAM
- Kernel creates contiguous virtual mapping
- Cannot use for hardware DMA
- No size limit (limited by total RAM)
- More overhead (page table changes, TLB invalidation)

**Cannot Use**:

- Interrupt context (requires page table allocation)
- DMA operations (hardware needs physical addresses)

---

### 10. kmalloc vs vmalloc Comparison

| Feature | kmalloc | vmalloc |
|---------|---------|---------|
| Contiguity | Physical + Virtual | Virtual only |
| Max Size | 4 MB | Physical RAM |
| Zone | LOWMEM | HIGHMEM |
| DMA | ✅ Yes | ❌ No |
| Interrupts | ✅ Yes (GFP_ATOMIC) | ❌ No |
| Overhead | Low | High |
| Performance | Fast | Slower |
| Use Case | Hardware, small | Large, kernel structures |

---

- **Best Practice**: Balance allocations/frees (exactly one kfree per kmalloc)

---

## KERNEL STACK MANAGEMENT

### Stack Size and Overflow Protection

**Linux Kernel Stack Size**:

- **Fixed size per process**: 8192 bytes (8 KB) on most systems
- **Shared**: Each task_struct has single kernel stack (no dynamic growth)
- **Danger**: Stack overflow is kernel panic (cannot extend like user stack)

**Protecting Stack from Overflow**:

```bash
# Compiler warning for large stack frames
grep CONFIG_FRAME_WARN /boot/config-$(uname -r)
# Default: CONFIG_FRAME_WARN=1024

# Means: compiler warns if any single function's stack frame exceeds 1024 bytes
# Set range: 0-8192 bytes (cannot exceed total 8192 byte kernel stack)

# Enable at compile time in Kconfig:
# config FRAME_WARN
#   int "Warn for stack frames larger than"
#   default 1024
#   range 0 8192
```

**What causes large stack frames?**

```c
// BAD: Large stack array in kernel function
void bad_kernel_function(void) {
    char buffer[4096];  // 4 KB local array - HALF the kernel stack!
    // Rest of function
}

// GOOD: Allocate from heap instead
void good_kernel_function(void) {
    char *buffer = kmalloc(4096, GFP_KERNEL);
    if (!buffer) return -ENOMEM;
    // Use buffer
    kfree(buffer);
}
```

**Check your kernel's setting**:

```bash
# View current CONFIG_FRAME_WARN value
cat /boot/config-$(uname -r) | grep FRAME_WARN

# To change: rebuild kernel with custom config
# Or use MODULE_PARAM in your module to accept runtime adjustment
```

---

## QUICK REFERENCE

**Decision Matrix**:

| Need | Solution | Limitation |
|------|----------|-----------|
| Small, fast, DMA | kmalloc(GFP_KERNEL) | ≤4 MB, ZONE_NORMAL |
| > 4 MB, not DMA | vmalloc() | Slower, ZONE_HIGHMEM (32-bit), cannot sleep |
| Needs zeros | kzalloc() | Same as kmalloc, ≤4 MB |
| In interrupt | kmalloc(GFP_ATOMIC) | ≤4 MB, cannot use vmalloc |
| Large local var | Avoid (use heap) | Max 1 KB per function (CONFIG_FRAME_WARN) |
| User app | malloc()/calloc() | Lazy allocation, can be paged |

**Common Commands**:

```bash
cat /proc/meminfo                  # Memory usage
cat /proc/buddyinfo               # Free pages by order
cat /proc/[pid]/maps              # Process memory layout
dmesg | head -20                  # Boot-time memory allocation
cat /boot/config-$(uname -r) | grep FRAME_WARN  # Stack protection setting
```

---

**Document Complete** - Comprehensive Linux Kernel Memory Management reference with all allocation functions, practical examples, and debugging tips.
