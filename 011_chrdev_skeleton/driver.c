#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_CNT 1
#define BASE_MINOR 0
#define DEVICE_NAME "my_dev"
#define CLASS_NAME "my_class"
#define BUF_SIZE 1024
#define DEVICE_PARENT NULL
#define DEVICE_DATA NULL

static dev_t devt;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static char buffer[BUF_SIZE];
static size_t buffer_size = 0;

#define print(fmt, ...) pr_info(fmt "\n", ##__VA_ARGS__)

static int my_open(struct inode *inode, struct file *file)
{
    print("device opened");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    print("device closed");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *pos)
{
    size_t available;

    if (*pos >= buffer_size)
        return 0;

    available = buffer_size - *pos;
    if (len > available)
        len = available;

    if (copy_to_user(buf, buffer + *pos, len))
        return -EFAULT;

    *pos += len;
    return len;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    size_t available;

    if (*pos >= BUF_SIZE)
        return -ENOSPC;

    available = BUF_SIZE - *pos;
    if (len > available)
        len = available;

    if (copy_from_user(buffer + *pos, buf, len))
        return -EFAULT;

    *pos += len;
    buffer_size = *pos;
    return len;
}

static const struct file_operations rw_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .llseek = default_llseek};

static int __init dev_init(void)
{
    int ret = alloc_chrdev_region(&devt, BASE_MINOR, DEVICE_CNT, DEVICE_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &rw_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, devt, DEVICE_CNT);
    if (ret < 0)
    {
        unregister_chrdev_region(devt, DEVICE_CNT);
        return ret;
    }

    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(devt, DEVICE_CNT);
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, DEVICE_PARENT, devt, DEVICE_DATA, DEVICE_NAME);
    if (IS_ERR(my_device))
    {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(devt, DEVICE_CNT);
        return PTR_ERR(my_device);
    }

    print("module loaded (major=%d, minor=%d)", MAJOR(devt), MINOR(devt));
    return 0;
}

static void __exit dev_exit(void)
{
    device_destroy(my_class, devt);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(devt, DEVICE_CNT);
    print("module unloaded");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");

module_init(dev_init);
module_exit(dev_exit);