# BeagleBone Black — TI Kernel Cross-Compile - Install

## 1. Required Library

```bash
sudo apt update
sudo apt install build-essential \
                 gcc-arm-linux-gnueabihf \
                 libncurses-dev \
                 bison \
                 flex \
                 libssl-dev \
                 libelf-dev \
                 bc \
                 git \
                 ca-certificates
```

**Library Usage:**

- `build-essential` — Compiler and build tools
- `gcc-arm-linux-gnueabihf` — ARM cross-compiler
- `libncurses-dev` — Terminal UI for menuconfig
- `bison` — Parser generator
- `flex` — Lexical analyzer
- `libssl-dev` — Cryptography library
- `libelf-dev` — ELF module support
- `bc` — Calculator utility
- `git` — Version control
- `ca-certificates` — SSL certificates

Create working directory:

```bash
mkdir -p /home/anis/linux
cd /home/anis/linux
```

---

## 2. Get TI Kernel Source

Download TI-optimized kernel source with real-time patches.

Find latest ti-based kernel for beagle bone:

```bash
git ls-remote --heads https://github.com/beagleboard/linux.git | grep ti-arm32
```

Clone repo with minimal history (faster download):

```bash
git clone --depth=1 --branch 6.6.58-ti-rt-arm32-r12 https://github.com/beagleboard/linux.git
cd linux
```

---

## 3. Configure for BeagleBone Black

Apply BeagleBone defaults:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig
```

**Output:** Creates `.config` file with kernel configuration settings for BeagleBone Black.

Optional — customize with menuconfig to enable WiFi driver:

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```

**To enable TP-Link Realtek WiFi (RTL8188EUS):**

Navigate in menuconfig:

```text
Device Drivers → Network device support → Wireless LAN → Realtek rtl8xxxu wireless driver
```

Select `<M>` to build as a module, then save and exit.

**Verify:**

```bash
grep RTL8XXXU .config
```

---

## 4. Build Kernel, DTBs, and Modules

Compiles the kernel and kernel modules based on the `.config` configuration file. This creates the bootable kernel image and device tree files.

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
     LOCALVERSION=-ti-rt-arm32-r12 \
     -j$(nproc) zImage dtbs modules
```

**Parameters:**

- `LOCALVERSION=-ti-rt-arm32-r12` — Kernel version suffix
- `-j$(nproc)` — Parallel compile jobs

**Outputs:**

- `arch/arm/boot/zImage` — Kernel image
- `arch/arm/boot/dts/am335x-boneblack.dtb` — Device tree
- Kernel modules — Driver modules

---

## 5. Install Modules into Home Directory

Copies compiled kernel modules to output directory.

```bash
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
     LOCALVERSION=-ti-rt-arm32-r12 \
     modules_install INSTALL_MOD_PATH=/home/anis/linux/output
```

**Output:**

```text
/home/anis/linux/output/lib/modules/6.6.58-ti-rt-arm32-r12/
```

---

## 6. Transfer to BeagleBone

Copy compiled kernel, device tree, and modules to BeagleBone using SCP.

Using root login and IP `192.168.7.2`:

```bash
# Kernel + DTB
scp arch/arm/boot/zImage root@192.168.7.2:/boot/vmlinuz-6.6.58-ti-rt-arm32-r12
scp arch/arm/boot/dts/ti/omap/am335x-boneblack.dtb root@192.168.7.2:/boot/dtbs/6.6.58-ti-rt-arm32-r12/am335x-boneblack-6.6.58.dtb

# Modules
Zip modules for faster transfer:

```bash
cd /home/anis/linux/output/lib/modules
tar -czf 6.6.58-ti-rt-arm32-r12.tar.gz 6.6.58-ti-rt-arm32-r12
```

Transfer zipped modules:

```bash
scp /home/anis/linux/output/lib/modules/6.6.58-ti-rt-arm32-r12.tar.gz \
    root@192.168.7.2:/lib/modules/
```

Extract on BeagleBone:

```bash
ssh root@192.168.7.2
cd /lib/modules
tar -xzf 6.6.58-ti-rt-arm32-r12.tar.gz
rm 6.6.58-ti-rt-arm32-r12.tar.gz
```
```

---

## 7. Update U-Boot Config

Configure boot loader to use new kernel and device tree.

On BeagleBone:

```bash
nano /boot/uEnv.txt
```

Set:

```ini
uname_r=6.6.58-ti-rt-arm32-r12
dtb=am335x-boneblack-6.6.58.dtb
```

---

## 8. Reboot & Verify

Restart BeagleBone and confirm new kernel is running.

```bash
sudo reboot
```

Verify kernel version:

```bash
uname -r
```

Expected:

```text
6.6.58-ti-rt-arm32-r12
```

---

## End
