# Linux File System Directory Structure

## 📂 Root Directory `/`

The starting point of the filesystem. Contains all other directories.

---

## 🔑 Essential Directories and Key Files

### 1. `/bin`

Essential user commands.

**Key files:**
- `ls` - List directory contents
- `cp` - Copy files
- `mv` - Move/rename files
- `rm` - Remove files
- `bash` - GNU Bourne Again Shell

---

### 2. `/sbin`

System administration commands.

**Key files:**
- `shutdown` - Power off system
- `mkfs` - Create filesystem
- `mount` - Mount filesystems
- `ifconfig` - Configure network interfaces

---

### 3. `/etc`

System configuration files.

**Key files:**
- `/etc/passwd` - User account information
- `/etc/shadow` - Encrypted user passwords
- `/etc/fstab` - Filesystem mount configuration
- `/etc/hosts` - Static hostname resolution
- `/etc/ssh/sshd_config` - SSH server configuration

---

### 4. `/dev`

Device files.

**Key files:**
- `/dev/sda` - First hard disk
- `/dev/null` - Discards all data written to it
- `/dev/tty` - Terminal devices
- `/dev/random` - Random number generator

---

### 5. `/proc`

Virtual filesystem for process and kernel info.

**Key files:**
- `/proc/cpuinfo` - CPU details
- `/proc/meminfo` - Memory usage
- `/proc/devices` - Registered devices
- `/proc/[pid]/` - Process-specific info

---

### 6. `/sys`

Virtual filesystem for devices and drivers.

**Key files:**
- `/sys/class/` - Device classes
- `/sys/devices/` - Hardware devices

---

### 7. `/lib`

Shared libraries and kernel modules.

**Key files:**
- `libc.so` - Standard C library
- `/lib/modules/` - Kernel modules

---

### 8. `/usr`

User applications and utilities.

**Key files:**
- `/usr/bin/` - Non-essential user commands (e.g., vim, gcc)
- `/usr/sbin/` - Non-essential system commands
- `/usr/share/man/` - Manual pages

---

### 9. `/var`

Variable data files.

**Key files:**
- `/var/log/syslog` - System log
- `/var/log/auth.log` - Authentication log
- `/var/spool/mail/` - User mailboxes

---

### 10. `/tmp`

Temporary files.

**Key files:**
- Application cache files
- Cleared on reboot

---

### 11. `/home`

User home directories.

**Key files:**
- `/home/username/.bashrc` - User shell configuration
- `/home/username/.ssh/` - SSH keys

---

### 12. `/root`

Root user's home directory.

**Key files:**
- `.bashrc` - Root shell configuration
- `.profile` - Root environment settings

---

### 13. `/boot`

Boot loader and kernel files.

**Key files:**
- `vmlinuz` - Linux kernel image
- `initrd.img` - Initial RAM disk
- `grub/grub.cfg` - GRUB bootloader configuration

---

### 14. `/opt`

Optional software packages.

**Key files:**
- Third-party applications installed here

---

### 15. `/media`

Mount points for removable media.

**Key files:**
- `/media/cdrom` - CD-ROM mount point
- `/media/usb` - USB mount point

---

### 16. `/mnt`

Temporary mount points.

**Key files:**
- Used for manual filesystem mounts

---

## 🧩 Special Directories and Files

### 17. `/run`

Runtime data.

**Key files:**
- PID files
- System sockets

---

### 18. `/srv`

Service data.

**Key files:**
- `/srv/www` - Web server files

---

### 19. `/lost+found`

Recovered files after filesystem check.

---

### 20. `/selinux`

SELinux configuration and status files.

---

## 📊 Summary Table

| Directory | Purpose | Key Files |
|-----------|---------|-----------|
| `/bin` | Essential user binaries | ls, bash |
| `/sbin` | System binaries | shutdown, mkfs |
| `/etc` | Configurations | passwd, fstab, hosts |
| `/dev` | Device files | sda, null, tty |
| `/proc` | Kernel/process info | cpuinfo, meminfo |
| `/sys` | Device info | class/, devices/ |
| `/lib` | Libraries | libc.so, modules |
| `/usr` | User apps | vim, gcc, man |
| `/var` | Variable data | syslog, auth.log |
| `/tmp` | Temporary files | Cache files |
| `/home` | User directories | .bashrc, .ssh/ |
| `/root` | Root's home | .bashrc, .profile |
| `/boot` | Boot files | vmlinuz, grub.cfg |
| `/opt` | Optional software | Third-party apps |
| `/media` | Removable media | cdrom, usb |
| `/mnt` | Mount points | Manual mounts |
| `/run` | Runtime data | PID files |
| `/srv` | Service data | www/ |
| `/lost+found` | Recovered files | fsck output |
| `/selinux` | SELinux configs | - |
