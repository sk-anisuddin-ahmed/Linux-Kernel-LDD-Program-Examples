#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "char_device"
#define CLASS_NAME "char"

struct char_device_data {
    char *buffer;
    size_t size;
};

int char_device_init(void);
void char_device_exit(void);
int char_device_open(struct inode *inode, struct file *file);
int char_device_release(struct inode *inode, struct file *file);
ssize_t char_device_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
ssize_t char_device_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);

#endif // CHAR_DEVICE_H