# BeagleBone Black Bring-Up via USB (Windows Host)

This guide explains how to set up internet connectivity on a BeagleBone Black (BBB) using USB connection from a Windows host, and prepare for building kernel modules.

**Note:** The default USB connection assigns IP `192.168.7.2` to the BBB. After configuring internet sharing (Step 2-3), the BBB will obtain an IP in the `192.168.137.x` range from Windows DHCP.

Default SSH access (before internet sharing setup): `ssh debian@192.168.7.2`

## Prerequisites

- **Board**: BeagleBone Black
- **Host OS**: Windows
- **Connection**: USB (RNDIS)
- **BBB OS**: Debian Buster (armhf)
- **Kernel Version**: 4.19.94-ti-r42

---

## Step 1: Connect BBB to Windows via USB

1. Connect your BeagleBone Black to your Windows computer using a USB cable.
2. The BBB should appear as a USB Ethernet device in Windows.

---

## Step 2: Configure Windows for Internet Sharing

1. On Windows, open the Network Connections window by pressing `Win + R`, typing `ncpa.cpl`, and pressing Enter.
2. Right-click your active internet connection (Wi-Fi or Ethernet) and select **Properties**.
3. Go to the **Sharing** tab.
4. Check the box for **Allow other network users to connect through this computer's Internet connection**.
5. From the dropdown, select the **USB Ethernet / RNDIS Gadget** (it might appear as "Ethernet 3" or similar).
6. Click **OK**.

### Windows USB Adapter Configuration

Windows will automatically assign:

- **IP Address**: 192.168.137.1
- **Subnet Mask**: 255.255.255.0
- **DHCP**: Enabled
- **NAT**: Enabled

---

## Step 3: Configure BBB Network Interface (usb0 with DHCP)

1. On the BBB, edit the network interfaces file:

   ```bash
   sudo nano /etc/network/interfaces
   ```

2. Make a backup of the original file (optional):

   ```bash
   sudo cp /etc/network/interfaces /etc/network/interfaces.old
   ```

3. Add or modify the usb0 interface configuration:

   ```ini
   auto usb0
   iface usb0 inet dhcp
   ```

4. Save and exit (Ctrl+X, Y, Enter).
5. Reboot the BBB:

   ```bash
   sudo reboot
   ```

---

## Step 4: Verify Network Configuration on BBB

### Check IP Address

Run:

```bash
ip addr show usb0
```

Expected output should show an IP like `192.168.137.x/24` (where x is a number assigned by DHCP).

### Check Routing

Run:

```bash
ip route
```

Expected output:

```
default via 192.168.137.1 dev usb0
192.168.137.0/24 dev usb0
```

### Verify from Windows Host

Run on Windows PowerShell:

```powershell
arp -a
```

Look for the BeagleBone's IP address in the 192.168.137.x range (it should appear as a dynamic entry with a physical address).

---

## Step 5: Set Up DNS on BBB

1. Edit the DNS configuration file:

   ```bash
   sudo nano /etc/resolv.conf
   ```

2. Make a backup of the original file (optional):

   ```bash
   sudo cp /etc/resolv.conf /etc/resolv.conf.old
   ```

3. Add the following nameservers:

   ```
   nameserver 8.8.8.8
   nameserver 8.8.4.4
   ```

4. Save and exit.

---

## Step 6: Test Internet Connectivity

1. Ping a public IP:

   ```bash
   ping 8.8.8.8
   ```

2. Ping a domain:

   ```bash
   ping google.com
   ```

Both should succeed if the setup is correct.

---

## Step 7: Clean Up Disk Space

1. Check current disk usage:

   ```bash
   df -h
   ```

2. Clean package cache:

   ```bash
   sudo apt-get clean
   sudo apt-get autoclean
   sudo apt-get autoremove --purge
   sudo rm -rf /var/lib/apt/lists/*
   ```

---

## Step 8: Fix Debian Buster (End-of-Life) Repositories

Since Debian Buster is EOL, update the sources to use archive repositories.

1. Edit the sources list:

   ```bash
   sudo nano /etc/apt/sources.list
   ```

2. Replace the entire content with:

   ```
   deb http://archive.debian.org/debian buster main contrib non-free
   deb http://archive.debian.org/debian-security buster/updates main contrib non-free
   deb [arch=armhf] http://repos.rcn-ee.com/debian buster main
   ```

3. Create or edit the APT configuration to disable expiry checks:

   ```bash
   sudo nano /etc/apt/apt.conf.d/99no-check-valid
   ```

4. Add these lines:

   ```
   Acquire::Check-Valid-Until "false";
   Acquire::AllowInsecureRepositories "true";
   ```

5. Update the package list:

   ```bash
   sudo apt-get update
   ```

---

## Step 9: Prepare for Kernel Module Build

1. Check your kernel version:

   ```bash
   uname -r
   ```

2. Install necessary build tools:

   ```bash
   sudo apt-get install build-essential bc kmod libncurses5-dev flex bison
   ```

---

## Step 10: Mount and Format SD Card

**Warning: This will erase all data on the SD card. Ensure you have backed up any important data.**

1. Identify the SD card device:

   ```bash
   lsblk
   ```

   Look for something like `/dev/mmcblk1` (the SD card slot; `/dev/mmcblk0` is the onboard eMMC).

2. Create a partition (if not already done):

   ```bash
   sudo fdisk /dev/mmcblk1
   ```

   - Press `n` for new partition
   - Press `p` for primary partition
   - Press `1` for partition number
   - Press Enter for default first sector
   - Press Enter for default last sector (uses entire disk)
   - Press `w` to write changes

3. Format the partition to ext4:

   ```bash
   sudo mkfs.ext4 /dev/mmcblk1p1
   ```

4. Create a mount point and mount:

   ```bash
   sudo mkdir -p /mnt/sdcard
   sudo mount /dev/mmcblk1p1 /mnt/sdcard
   ```

5. Verify:

   ```bash
   df -h | grep sdcard
   ```

---

## Step 11: Download Kernel Source/Headers

1. Navigate to the SD card:

   ```bash
   cd /mnt/sdcard
   ```

2. Download the kernel source archive for your version:

   ```bash
   wget https://github.com/beagleboard/linux/archive/refs/tags/4.19.94-ti-r42.tar.gz
   ```

3. Extract:

   ```bash
   tar -xvzf 4.19.94-ti-r42.tar.gz
   cd linux-4.19.94-ti-r42
   ```

---

## Step 12: Prepare Kernel Source

1. Configure for BeagleBone:

   ```bash
   make ARCH=arm bb.org_defconfig
   ```

2. Prepare for external modules:

   ```bash
   make ARCH=arm modules_prepare
   ```

3. Export headers (optional, for user-space programs):

   ```bash
   make ARCH=arm headers_install INSTALL_HDR_PATH=/usr
   ```

---

## Step 13: Create Symlink for Build Directory

1. Point the kernel build path to your prepared source tree:

   ```bash
   sudo ln -sf /mnt/sdcard/linux-4.19.94-ti-r42 /lib/modules/$(uname -r)/build
   ```

2. Verify:

   ```bash
   ls -l /lib/modules/$(uname -r)/build
   ```

---

## Step 14: Build a Test Module (hello.ko)

1. Create a test directory:

   ```bash
   mkdir ~/hello && cd ~/hello
   ```

2. Write `hello.c`:

   ```c
   #include <linux/module.h>
   #include <linux/kernel.h>

   int init_module(void) {
       printk(KERN_INFO "Hello, kernel!\n");
       return 0;
   }

   void cleanup_module(void) {
       printk(KERN_INFO "Goodbye, kernel!\n");
   }

   MODULE_LICENSE("GPL");
   ```

3. Write `Makefile`:

   ```makefile
   obj-m += hello.o

   all:
       make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

   clean:
       make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
   ```

4. Build:

   ```bash
   make
   ```

   This produces `hello.ko`.

---

## Step 15: Test the Module

1. Insert the module:

   ```bash
   sudo insmod hello.ko
   ```

2. Check logs:

   ```bash
   dmesg | tail
   ```

   Should show:

   ```
   Hello, kernel!
   ```

3. Remove the module:

   ```bash
   sudo rmmod hello
   dmesg | tail
   ```

   Should show:

   ```
   Goodbye, kernel!
   ```

With these steps, you now have a complete workflow: Format/mount SD card → download kernel source → prepare tree → symlink → build modules → test with hello.ko.
