#include <linux/module.h>  // Needed by all modules
#include <linux/init.h>    // For module_init/module_exit macros
#include <linux/fs.h>      // For file_operations, alloc_chrdev_region
#include <linux/cdev.h>    // For cdev_init, cdev_add, struct cdev
#include <linux/device.h>  // For class_create, device_create
#include <linux/uaccess.h> // For copy_to_user, copy_from_user
#include <linux/kernel.h>  // For printk, pr_info
#include <linux/types.h>   // For dev_t, size_t, etc.
#include <linux/wait.h>    // For wait_event_interruptible, wake_up_interruptible

#define base_minor 0
#define sys_parent_obj NULL
#define private_data NULL

#define device_cnt 1
#define device_name "my_dev"
#define class_name "my_class"
#define READ_TIMEOUT (5 * HZ) // 5 second timeout

static dev_t devt;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static DECLARE_WAIT_QUEUE_HEAD(wq);
static char data[256];
static int data_ready = 0;
static int readers_count = 0;
static int total_readers = 2;

#define print(fmt, ...) pr_info(fmt "\n", ##__VA_ARGS__)

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *pos)
{
    long ret = wait_event_interruptible_timeout(wq, data_ready, READ_TIMEOUT);

    if (ret < 0)
        return -ERESTARTSYS;
    if (ret == 0)
    {
        print("Read timeout");
        return -ETIMEDOUT;
    }

    len = strlen(data);
    if (copy_to_user(buf, data, len))
        return -EFAULT;

    readers_count++;
    print("Reader: %d/%d", readers_count, total_readers);

    if (readers_count >= total_readers)
    {
        readers_count = 0;
        data_ready = 0;
        wake_up_interruptible(&wq);
    }

    return len;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    if (len > 255)
        len = 255;

    if (copy_from_user(data, buf, len))
        return -EFAULT;

    data[len] = '\0';
    data_ready = 1;
    readers_count = 0;

    wake_up_interruptible(&wq);
    print("Data written: %s", data);

    return len;
}

static const struct file_operations rw_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
};

static int __init dev_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&devt, base_minor, device_cnt, device_name);
    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &rw_fops);
    my_cdev.owner = THIS_MODULE;
    cdev_add(&my_cdev, devt, device_cnt);

    my_class = class_create(THIS_MODULE, class_name);
    my_device = device_create(my_class, sys_parent_obj, devt, private_data, device_name);

    print("module loaded");
    return 0;
}

static void __exit dev_exit(void)
{
    device_destroy(my_class, devt);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(devt, device_cnt);
    print("module unloaded");
}

MODULE_LICENSE("GPL");

module_init(dev_init);
module_exit(dev_exit);