# Booting in Embedded Systems and BeagleBone Black

Booting refers to the sequence of steps a computing system follows to transition from a powered-off state to a fully operational environment. In embedded systems, this process is carefully staged because hardware resources are limited and reliability is critical. The BeagleBone Black, based on the Texas Instruments AM335x ARM processor, provides a concrete example of how embedded Linux booting works.

---

## General Booting in Computers

All computer systems follow a common boot sequence:

1. **Power-on reset**: Hardware initializes to a known state
2. **Firmware/ROM code**: First immutable program executed by the processor
3. **Bootloader(s)**: Intermediate loaders prepare hardware and load the OS
4. **Kernel**: Core OS that manages memory, processes, and devices
5. **Filesystem and Init**: Mounts storage and starts user-space services

This layered approach is necessary because hardware cannot directly execute a full operating system—each stage provides greater capability to bridge the gap.

---

## Embedded Booting

Embedded systems follow the same principles but with unique constraints:

- **ROM code**: Immutable, provides guaranteed start point
- **Multi-stage bootloaders**: SPL (first stage) initializes RAM, then U-Boot provides full control
- **Device Tree**: Replaces hard-coded hardware configuration
- **Minimal footprint**: Code and data strictly limited by on-chip SRAM

Embedded boot design priorities:

- **Determinism**: Predictable, reliable startup sequences
- **Customization**: Only required components are loaded
- **Recovery**: Robust fallback mechanisms for failed boots
- **Efficiency**: Minimizes power consumption and boot time

---

## BeagleBone Black Boot Flow

The BeagleBone Black illustrates embedded booting in practice:

1. **ROM Code (SoC internal)**  
   - Stored permanently in the AM335x processor
   - Reads boot pins to determine boot order (eMMC, SD, USB, UART)
   - Loads first-stage bootloader (MLO) into SRAM
   - Role: Provides immutable boot anchor regardless of storage state

2. **SPL / MLO (First Stage Bootloader)**  
   - Minimal loader (~64 KB) stored in boot partition of eMMC or SD
   - Executes in on-chip SRAM; initializes DDR3 memory controller
   - Loads full bootloader (U-Boot) into initialized DDR
   - Critical: Enables access to main memory; without it, DDR remains unconfigured

3. **U-Boot (Second Stage Bootloader)**  
   - Executes from DDR RAM; reads `/boot/uEnv.txt` for user configuration
   - Loads kernel and device tree; passes kernel command line (rootfs location, console)
   - Enables boot from multiple sources; supports recovery and flashing operations

4. **Linux Kernel**  
   - Decompressed into RAM; initializes CPU, MMU, memory management, and device drivers
   - Parses device tree to discover and configure hardware interfaces
   - Mounts root filesystem and transitions to user-space execution

5. **Device Tree Blob (DTB)**  
   - Hardware description file (e.g., `am335x-boneblack.dtb`)
   - Describes GPIOs, I²C, SPI, Ethernet, clock configuration, interrupt mappings
   - Enables single kernel binary to support multiple board configurations without recompilation

6. **Root Filesystem (RootFS)**  
   - Mounted from persistent storage (eMMC or SD card, typically ext4)
   - Contains system binaries, libraries, configuration files, and user applications
   - Survives power cycles; enables persistent state and software ecosystem

7. **Init System (systemd on Debian)**  
   - First user-space process; manages service dependencies and startup sequence
   - Starts essential services: udev (device management), networking, SSH, logging
   - Presents login prompt or GUI; completes transition to operational state

---

## Storage Partitions and Boot Files

Understanding where boot components live on storage is critical for understanding boot failures, updates, and recovery.

### BeagleBone Black Partition Layout (eMMC or SD Card)

**Typical ext4-based layout**:

```
Sector 0       | Master Boot Record (MBR) or GPT header
Sectors 1-256  | MLO (First Stage Bootloader, ~64-256 KB)
               | ROM code searches here first; name must be "MLO"
Sectors 257+   | U-Boot binary (usually named "u-boot.img", ~500 KB-1 MB)
               | Loaded by MLO into DDR RAM
/boot          | Kernel partition (ext4) containing:
               |  - zImage (kernel image, ~4-6 MB compressed)
               |  - am335x-boneblack.dtb (device tree, ~50-100 KB)
               |  - uEnv.txt (U-Boot environment/configuration)
               |  - boot.scr (optional: U-Boot boot script)
/               | Root filesystem (ext4) containing:
               |  - /bin, /etc, /usr, /lib, /home, etc.
               |  - Entire Linux OS and applications
```

**Critical file names**:

- `MLO`: Must be named exactly this (case-sensitive); ROM code searches by this name
- `u-boot.img`: U-Boot binary; loaded by MLO
- `zImage` or `vmlinuz`: Compressed kernel
- `am335x-boneblack.dtb`: Device tree for this specific board
- `uEnv.txt`: U-Boot environment; controls boot behavior

**uEnv.txt example** (controls what U-Boot does):

```
bootdir=/boot
bootfile=zImage
fdtfile=am335x-boneblack.dtb
console=ttyO0,115200n8
root=/dev/mmcblk0p2 rw
rootfstype=ext4
```

---

### Generic ARM (Yocto, Buildroot projects)

**Typical partition layout**:

```
Sectors 0-255    | SPL (16-64 KB, name varies: MLO, u-boot-spl.bin)
Sectors 256+     | U-Boot (usually 400-600 KB)
Partition 1      | Boot partition:
                 |  - kernel image (zImage, uImage, or fit image)
                 |  - Device tree blobs
                 |  - boot.scr or environment files
Partition 2      | Root filesystem (ext4, squashfs, or ubifs)
```

**Alternative: NAND flash layout** (high-volume embedded):

```
Block 0         | SPL (must fit in minimal space)
Block 1+        | U-Boot
UBIFS partition | Kernel + DTB (using UBI for wear-leveling)
UBIFS partition | Root filesystem
```

**File naming conventions**:

- `u-boot-spl.bin`: SPL stage bootloader
- `u-boot.bin`: U-Boot binary
- `uImage`: Kernel wrapped in legacy U-Boot header (older systems)
- `fit.itb`: Flattened Image Tree (newer, combines kernel + DTB + scripts)
- Device tree: `device-tree.dtb`, `fdt.dtb`, or board-specific name

---

### Raspberry Pi Partition Layout (microSD Card)

**Unique layout** (GPU firmware manages boot):

```
Sectors 0-63     | MBR (not used; ROM ignores it)
Sectors 64-8192  | Fat32 BOOT partition (visible on Windows)
                 |  - bootcode.bin (GPU bootloader, ~6 KB)
                 |  - start.elf (GPU firmware, ~1-2 MB)
                 |  - fixup.dat (GPU memory initialization)
                 |  - config.txt (boot parameters, readable text file)
                 |  - cmdline.txt (kernel command line)
                 |  - kernel.img or kernel7.img (kernel binary)
                 |  - overlays/ (device tree overlays)
Sectors 8192+    | Ext4 ROOT partition
                 |  - /bin, /etc, /usr, /lib, etc.
```

**Critical differences from standard ARM**:

- ROM code directly reads from FAT32 boot partition
- GPU firmware (start.elf) must be present; ROM halts without it
- No U-Boot by default; boot parameters in human-readable config.txt
- Kernel filename determines which CPU variant runs (kernel.img vs kernel7.img)

**config.txt controls**:

```
gpu_mem=64              # GPU memory allocation
arm_freq=1000           # CPU frequency
sdram_freq=500          # Memory frequency
dtoverlay=gpio-button   # Load device tree overlay
```

---

### x86/x64 Partition Layout (BIOS/UEFI)

**BIOS Legacy boot**:

```
Sector 0       | Master Boot Record (MBR) - contains boot code
Sectors 1-63   | Unused/Reserved
Partition 1    | /boot (ext4 or ext3) containing:
               |  - vmlinuz (kernel image, ~5-8 MB)
               |  - initrd.img (initial ramdisk, loaded before rootfs)
               |  - GRUB configuration and modules
Partition 2    | / (root filesystem, ext4, btrfs, etc.)
               | - Entire OS: /bin, /etc, /usr, /home, /var
```

**UEFI boot** (modern systems):

```
Partition 1    | EFI System Partition (FAT32) containing:
               |  - EFI/BOOT/BOOTX64.EFI (UEFI bootloader)
               |  - EFI/ubuntu/ or vendor-specific directories
               |  - GRUB UEFI bootloader and modules
Partition 2    | /boot (optional, may be within root partition)
Partition 3    | / (root filesystem, usually ext4 or btrfs)
```

**File naming conventions**:

- `vmlinuz-5.10.0-generic`: Kernel with version
- `initrd.img-5.10.0-generic`: Initial ramdisk
- `grub/grub.cfg`: GRUB configuration (text file)
- `BOOTX64.EFI`: UEFI bootloader binary

---

### Partition Table Tools

**How ROM code finds boot files**:

**ARM embedded (BeagleBone Black)**:

```
ROM Code:
1. Reads boot pins (determine boot source: SD, eMMC, USB)
2. Searches for file named "MLO" in first accessible sectors
3. Loads MLO into on-chip SRAM and executes
4. MLO initializes DDR, reads partition table, loads U-Boot
5. U-Boot reads filesystem, finds uEnv.txt, loads kernel+DTB
```

**Raspberry Pi (Broadcom)**:

```
ROM Code:
1. Hard-coded to boot from microSD card
2. Reads FAT32 boot partition
3. Loads bootcode.bin, then start.elf (GPU firmware)
4. GPU firmware reads config.txt, loads kernel and DTB
5. Transitions to CPU execution
```

**x86 (BIOS)**:

```
ROM Code (BIOS):
1. Checks MBR in sector 0 for boot signature (0xAA55)
2. Executes boot code from MBR
3. GRUB bootloader reads partition table, finds /boot
4. Loads kernel and initrd from /boot partition
5. Passes control to kernel
```

**x86 (UEFI)**:

```
ROM Code (UEFI):
1. Reads EFI System Partition (FAT32)
2. Locates BOOTX64.EFI or vendor-specific boot file
3. Executes UEFI bootloader (GRUB or alternative)
4. Bootloader reads /boot or root partition, loads kernel
5. Passes control to kernel with EFI runtime services available
```

---

## Partition Manipulation and Recovery

**Viewing partitions on Linux**:

```bash
# List partitions
lsblk
parted -l
fdisk -l

# Mount boot partition to inspect files
mount /dev/mmcblk0p1 /mnt/boot
ls /mnt/boot/   # See MLO, u-boot.img, kernel, DTB
cat /mnt/boot/uEnv.txt  # View U-Boot configuration
```

**Common boot failures and causes**:

- **MLO missing or corrupted**: ROM code halts; device unresponsive
- **DDR initialization failed**: SPL crashes silently; no console output
- **Kernel image missing**: U-Boot prompt appears but boot fails
- **Device tree mismatch**: Kernel panics if DTB doesn't match board
- **Partition corruption**: Filesystem errors; system won't mount
- **Bootloader config wrong**: System boots but to wrong root device

---

## Platform Comparison: Boot Flows Across Architectures

### BeagleBone Black (ARM Cortex-A8, TI AM335x)

**Problem it solves**: Affordable, industrial-grade embedded computing with real-time capabilities and expansion headers.

**Boot flow**: ROM Code → SPL/MLO → U-Boot → Kernel → DTB → RootFS → systemd

**Significance and Role**:

- Multi-stage bootloader required due to 64 KB on-chip SRAM limit
- DTB (device tree blob) abstracts hardware, enabling single kernel for many board variants
- U-Boot provides extensive debugging, network boot, and recovery capabilities
- Deterministic boot timing critical for real-time applications
- Low power consumption (1-2W idle) enables battery-backed or solar deployments

**Unique characteristics**:

- ROM code reads boot pins; supports eMMC, microSD, USB, or UART boot
- SPL (~50-70 KB) handles DDR controller configuration; must fit in SRAM
- U-Boot can load over Ethernet (TFTP) for development and mass deployment
- Device tree allows runtime hardware configuration changes without kernel recompilation

---

### Raspberry Pi (ARM Cortex-A72/A53, Broadcom SoC)

**Problem it solves**: Ultra-low-cost entry point for Linux education, IoT, and hobbyist projects.

**Boot flow**: ROM Code → Bootcode.bin → Start.elf → U-Boot/NOOBS → Kernel → RootFS → systemd

**Significance and Role**:

- Simplified boot process hides complexity; users rarely interact with bootloader directly
- GPU firmware (start.elf) executes before CPU; manages clock speeds and DDR initialization
- Proprietary ROM code differs significantly from standard ARM implementations
- MicroSD card bootloader is user-accessible and easily customizable
- Targets mass-market users without embedded Linux expertise

**Unique characteristics**:

- Broadcom SoC contains GPU alongside ARM CPU; both boot in parallel
- config.txt file (on microSD) controls boot parameters instead of U-Boot prompt
- OTP (One-Time Programmable) memory stores manufacturing settings
- NOOBS (Raspberry Pi's installer) handles multi-OS selection and automated setup
- Limited recovery options compared to industrial platforms; SD card corruption is common failure mode

---

### Generic ARM Embedded Systems (Cortex-A, Cortex-M)

**Problem it solves**: Flexible, customizable embedded Linux for industrial, automotive, and IoT applications.

**Boot flow**: ROM Code → SPL (or minimal bootloader) → U-Boot → Kernel → DTB → RootFS → systemd

**Significance and Role**:

- Standard approach adopted by manufacturers needing Linux with real-time or safety requirements
- Device tree widely expected; enables IP reuse across product lines
- Bootloader remains visible to developers for debugging, recovery, and field updates
- Supports both RTOS (real-time) and general-purpose Linux variants

**Unique characteristics**:

- Boot pins configure boot source, clock multipliers, and security settings
- SPL often uses minimal initialization (can be omitted if DDR pre-initialized)
- Multi-partition support: boot partition separate from root filesystem
- NAND flash boot common in high-volume products (UBIFS or UBI file system)
- Secure boot: ARM TrustZone enables signed kernel and encrypted partitions

---

### x86/x64 (Intel/AMD Desktop, Server, Industrial)

**Problem it solves**: High-performance, general-purpose computing with extensive driver ecosystem and enterprise support.

**Boot flow**: BIOS/UEFI → GRUB (or UEFI stub kernel) → Kernel → RootFS → systemd

**Significance and Role**:

- Largest software and driver ecosystem; supports thousands of peripheral types
- ACPI (Advanced Configuration and Power Interface) replaces device trees for hardware discovery
- Multi-boot capable: single bootloader manages Windows, Linux, BSD simultaneously
- Enterprise features: TPM (Trusted Platform Module), Secure Boot, remote management

**Unique characteristics**:

- BIOS (legacy, real mode) or UEFI (modern, protected mode) firmware; often 10+ MB in size
- GRUB bootloader is much more complex than U-Boot; ~1.5 MB typical
- Memory detection automatic via hardware probing; device tree not needed
- CPU remains active throughout boot process (unlike ARM SPL where CPU may idle)
- Firmware stores boot settings in CMOS or NVRAM; survives power loss
- Graphics output during boot; splash screens and boot menus expected
- Power consumption irrelevant compared to embedded systems; single-digit watts during boot is acceptable

---

### Comparison Matrix

| Feature | BeagleBone Black | Raspberry Pi | Generic ARM | x86/x64 |
|---------|------------------|--------------|------------|---------|
| **ROM Code** | TI-specific | Broadcom proprietary | Standard ARM | Intel/AMD BIOS/UEFI |
| **Bootloader** | U-Boot | Broadcom firmware + optional U-Boot | U-Boot | GRUB |
| **Hardware Config** | Device Tree | GPU firmware + config.txt | Device Tree | ACPI tables |
| **On-chip SRAM** | 64 KB (critical limit) | 128 KB (less constrained) | Varies 32-512 KB | N/A (DDR always available) |
| **Boot Sources** | eMMC, SD, USB, UART, Ethernet | microSD only (ROM hardcoded) | SD, NAND, eMMC, USB, Ethernet | HDD, SSD, USB, Network (PXE) |
| **User Control** | High (U-Boot prompt) | Low (config.txt only) | High (U-Boot) | Medium (GRUB menu) |
| **Development** | Industrial/Real-time | Hobbyist/Education | Industrial/Enterprise | General-purpose/Enterprise |
| **Typical Boot Time** | 2-5 seconds | 5-10 seconds | 2-5 seconds | 10-30 seconds |
| **Power Idle** | 0.5-1 W | 1-2 W | 0.5-2 W | 10-50 W |
| **Secure Boot** | Optional (TrustZone) | Not standard | Optional (TrustZone) | Standard (Secure Boot) |

---

## Why These Differences Exist

**ARM Embedded (BBB, Generic ARM)**

- Problem: Limited on-chip RAM requires multi-stage bootloader strategy
- Solution: SPL initializes DDR in minimal code; U-Boot runs from DDR with full features
- Benefit: Determinism, low power, customizable boot chain

**Raspberry Pi**

- Problem: Target non-technical users who shouldn't see a bootloader
- Solution: GPU firmware abstracts hardware initialization; simplified boot
- Benefit: Low entry barrier, automatic clock scaling, beginner-friendly

**x86 Desktop/Server**

- Problem: Must support heterogeneous hardware (thousands of device types)
- Solution: Firmware autodetects hardware; bootloader handles OS selection
- Benefit: Universal compatibility, plug-and-play peripherals, multi-boot capability

---

## Summary

Booting follows a universal principle: **ROM/BIOS → Bootloader(s) → Kernel → Filesystem → Services**, but implementations vary dramatically based on design goals.

**ARM Embedded systems** (BeagleBone Black, generic ARM) optimize for determinism, power efficiency, and customization. Multi-stage bootloaders and device trees enable predictable, flexible initialization even with limited resources.

**Raspberry Pi** targets ease-of-use by abstracting bootloader complexity. GPU firmware pre-initializes the system; users interact only with simple config files, not bootloaders.

**x86 Systems** prioritize universality and performance. Larger firmware handles auto-detection; more complex bootloaders support arbitrary device configurations and operating system choices.

Each architecture solves a distinct problem set: embedded systems need determinism and efficiency; consumer platforms need simplicity; servers need flexibility and compatibility. The boot sequence reflects these priorities.
