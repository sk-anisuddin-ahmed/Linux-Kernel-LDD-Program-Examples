# Virtual Memory and Page Tables

Virtual memory is a fundamental abstraction that decouples logical (virtual) address spaces from physical memory. This mechanism enables efficient memory management, process isolation, and hardware abstraction across all modern operating systems. Page tables are the hardware structures that implement this translation.

---

## Pages and Frames

**Page**: A fixed-size block of virtual address space (typically 4 KB on modern systems).

**Frame**: A fixed-size block of physical RAM, identical in size to a page.

**Virtual-to-Physical Mapping**: Page tables maintain the translation from virtual pages to physical frames. Each virtual page can map to any physical frame, or remain unmapped (causing a page fault).

---

## The Multi-Level Page Table Problem

### Single-Level Page Table (Inefficient)

For a 64-bit system with 4 KB pages:

- Virtual address space: 2^64 bytes = 16 exabytes
- Page size: 4 KB = 2^12 bytes
- Required entries: 2^64 / 2^12 = 2^52 entries
- Memory for one process: 2^52 × 8 bytes = 32 petabytes

This is physically impossible. A flat page table cannot scale.

### Multi-Level Solution

Hierarchical page tables break the address space into smaller chunks:

- Only allocate page table levels that are actually used
- Sparse address spaces (with gaps) consume minimal memory
- Typical process uses only 1-2% of theoretical address space

---

## Linux Page Table Hierarchy (x86-64)

Linux uses a 4-level hierarchical structure on x86-64 processors:

| Level | Name | Size | Purpose |
|-------|------|------|---------|
| 1 | **PGD** (Page Global Directory) | 512 entries (4 KB) | Top-level directory; typically one per process |
| 2 | **PUD** (Page Upper Directory) | 512 entries (4 KB) | Covers 512 GB regions of address space |
| 3 | **PMD** (Page Middle Directory) | 512 entries (4 KB) | Covers 1 GB regions of address space |
| 4 | **PTE** (Page Table Entry) | 512 entries (4 KB) | Final level; points to physical frame |

**Architecture analogy**: PGD = filing cabinet → PUD = drawer → PMD = folder → PTE = individual file card with physical frame address.

---

## Virtual Address Translation

When the CPU executes an instruction referencing virtual memory, the MMU (Memory Management Unit) performs address translation:

### Translation Steps

1. **Extract page table indices**: CPU splits 64-bit virtual address into components:
   - PGD index: bits 39-47 (9 bits) → selects entry in PGD
   - PUD index: bits 30-38 (9 bits) → selects entry in PUD
   - PMD index: bits 21-29 (9 bits) → selects entry in PMD
   - PTE index: bits 12-20 (9 bits) → selects entry in PTE
   - Offset: bits 0-11 (12 bits) → offset within 4 KB page

2. **Walk the hierarchy**:

   ```
   PGD + (PGD_index × 8) → PUD address
   PUD + (PUD_index × 8) → PMD address
   PMD + (PMD_index × 8) → PTE address
   PTE + (PTE_index × 8) → frame number
   ```

3. **Construct physical address**:

   ```
   Physical address = (frame_number << 12) + offset
   ```

### Performance: TLB Caching

Walking four levels on every memory access would be catastrophically slow. Modern CPUs include a **Translation Lookaside Buffer (TLB)**:

- Hardware cache of recent virtual-to-physical translations
- Hit rate: 95-99% on typical workloads
- On TLB miss: CPU walks full hierarchy (expensive)
- Context switch: TLB must be flushed or tagged with ASID (Address Space Identifier)

---

## Page Table Entry (PTE) Structure

Each PTE is typically 64 bits on modern systems, containing:

| Bits | Purpose |
|------|---------|
| 0 | **P (Present)**: Page is in physical RAM (1) or swapped to disk (0) |
| 1 | **R/W (Read/Write)**: 1 = writable; 0 = read-only |
| 2 | **U/S (User/Supervisor)**: 1 = user-accessible; 0 = kernel-only |
| 3 | **PWT (Page Write-Through)**: Cache write behavior |
| 4 | **PCD (Page Cache Disable)**: Disable caching for this page |
| 5 | **A (Accessed)**: Set when page is read (used by OS) |
| 6 | **D (Dirty)**: Set when page is written (used for paging) |
| 7-8 | Reserved |
| 9-11 | **Available**: OS-specific flags |
| 12-51 | **Frame Number**: Physical address >> 12 (40 bits, supports 1 TB RAM) |
| 52-62 | Reserved/Protection keys |
| 63 | **XD/NX (No-Execute)**: 1 = page is non-executable |

### Protection Mechanism

- **Present bit = 0**: Access triggers page fault; OS can:
  - Load page from disk (if swapped)
  - Allocate new zero-filled page
  - Deliver SIGSEGV (segmentation fault) if unmapped
  
- **R/W bit = 0**: Write attempt triggers page fault; OS can:
  - Copy page for copy-on-write optimization
  - Deliver SIGSEGV if region is read-only

- **NX bit = 1**: Execution attempt triggers fault; enables DEP (Data Execution Prevention)

---

## Virtual Address Space Layout

Modern systems divide virtual address space into user and kernel regions:

### 64-bit x86-64 Layout

```
Address Range           | Access | Purpose
-----------             |--------|----------
0x0000000000000000      | User   | Text (code)
0x0000000000400000      | User   | Initialized data
0x0000000000600000      | User   | Uninitialized data (BSS)
0x0000000010000000      | User   | Heap (grows upward)
                        |        |
(gap)                   |        | Unmapped
                        |        |
0x00007ffffffde000      | User   | Stack (grows downward)
0x00007ffff7dce000      | User   | Memory-mapped regions, shared libraries
0x00007ffffffff000      | User   | Final user-accessible address
-----------             |--------|----------
0xffff800000000000      | Kernel | Kernel text and data
0xffffffff82000000      | Kernel | Loaded modules
0xffffffffffffffff      | Kernel | Final kernel address
```

**Key observations**:

- User space: Lower half (0x0... to 0x7fff...)
- Kernel space: Upper half (0xffff8000... to 0xffff...)
- Heap: Grows upward from code segment
- Stack: Grows downward from top of user space
- Gap between stack and heap: Guard region (prevents collision)
- Each process has isolated user space (different PGD)
- Kernel space shared across all processes

---

## Shared Memory and Copy-on-Write

### Shared Memory

Multiple processes can map the same physical frame:

- Shared libraries (libc.so): Single copy in RAM, multiple PTE entries
- Shared data: Intentional inter-process communication
- PTE flags protect shared regions from unintended modification

### Copy-on-Write (CoW)

Process forking optimization:

1. **After fork()**: Child inherits parent's PGD; both share physical frames
2. **R/W bit = 0**: Both read-only copies initially
3. **Write attempt**: Page fault triggers; kernel:
   - Allocates new physical frame
   - Copies page data
   - Updates child's PTE to point to new frame
   - Sets R/W bit = 1
   - Resumes execution

**Benefit**: fork() is O(1) instead of O(n); expensive copy deferred until actual modification occurs.

---

## Page Faults and Kernel Handling

Page faults occur when CPU attempts to access unmapped or protected memory:

### Fault Types

1. **Minor Fault**: Page exists but not in TLB
   - Kernel reloads TLB entry; very fast recovery

2. **Major Fault**: Page swapped to disk
   - Kernel reads from swap device to physical RAM
   - Re-maps PTE; cost: 10-100 ms (milliseconds)

3. **Invalid Fault**: Access to unmapped region
   - Kernel delivers SIGSEGV signal
   - Process terminates (or catches signal)

### Page Fault Handler

```
Page Fault:
1. Determine faulting address
2. Look up in VMA (Virtual Memory Area) tree
3. If VMA found:
   - Check access permissions
   - Allocate new frame or load from disk
   - Update PTE with new frame number
   - Flush TLB entry
   - Resume execution
4. If VMA not found:
   - Send SIGSEGV to process
```

---

## Comparison: x86-64 vs ARM64 vs RISC-V

| Feature | x86-64 | ARM64 | RISC-V |
|---------|--------|-------|--------|
| **Page sizes** | 4 KB, 2 MB, 1 GB | 4 KB, 16 KB, 64 KB, 2 MB | 4 KB, 2 MB, 1 GB |
| **Page table levels** | 4 | 3-4 (configurable) | 3 (configurable) |
| **TLB types** | I-TLB, D-TLB | Unified | Unified |
| **ASID support** | No (full flush on context switch) | Yes (fine-grain TLB tagging) | Yes (ASID extension) |
| **PTE size** | 8 bytes (64-bit) | 8 bytes (64-bit) | 8 bytes (64-bit) |
| **NUMA support** | Yes (inter-socket) | Limited | Limited |
| **Huge pages** | Yes (2 MB, 1 GB) | Yes (various sizes) | Yes (configurable) |

---

## Summary

Virtual memory is essential for modern operating systems:

- **Abstraction**: Processes see unlimited virtual address space independent of physical RAM
- **Isolation**: Each process has independent page tables; memory access is protected
- **Efficiency**: Multi-level hierarchies scale to large address spaces without excessive memory overhead
- **Flexibility**: Page-level granularity enables fine-grained access control, sharing, and paging strategies

Page table hardware (MMU + TLB) bridges the gap between software address spaces and physical memory, enabling secure, efficient, and scalable memory management across diverse computing systems.
