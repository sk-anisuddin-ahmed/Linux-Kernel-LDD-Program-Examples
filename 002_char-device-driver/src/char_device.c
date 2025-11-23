#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define DEVICE_NAME "my_char_device"
#define BUFFER_SIZE 1024

static int major_number;
static char message[BUFFER_SIZE] = {0};
static short size_of_message;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .open = device_open,
    .read = device_read,
    .write = device_write,
    .release = device_release,
};

static int __init char_device_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        return major_number;
    }
    return 0;
}

static void __exit char_device_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
}

static int device_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int device_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t device_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    int error_count = copy_to_user(buffer, message, size_of_message);
    if (error_count == 0) {
        return size_of_message;
    } else {
        return -EFAULT;
    }
}

static ssize_t device_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    snprintf(message, BUFFER_SIZE, "%s", buffer);
    size_of_message = strlen(message);
    return len;
}

module_init(char_device_init);
module_exit(char_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("0.1");