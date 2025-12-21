#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#define TEMP_SET_HIGH     _IOW('A', 1, int)
#define TEMP_SET_LOW      _IOW('B', 2, int)
#define TEMP_GET_CURRENT  _IOR('C', 3, int)
#define TEMP_GET_ALERTS   _IOR('D', 4, int)

struct temp_sensor {
    atomic_t current_temp;
    atomic_t high_threshold;
    atomic_t low_threshold;
    atomic_t alert_count;
};

static struct temp_sensor sensor;
static dev_t temp_dev;
static struct cdev temp_cdev;
static struct class *temp_class;
static struct device *temp_device;

static long temp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int val;

    switch (cmd) {
    case TEMP_SET_HIGH:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        atomic_set(&sensor.high_threshold, val);
        return 0;

    case TEMP_SET_LOW:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        atomic_set(&sensor.low_threshold, val);
        return 0;

    case TEMP_GET_CURRENT:
        val = atomic_read(&sensor.current_temp);
        if (copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        return 0;

    case TEMP_GET_ALERTS:
        val = atomic_read(&sensor.alert_count);
        if (copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        return 0;

    default:
        return -EINVAL;
    }
}

static ssize_t temp_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int new_temp;
    if (copy_from_user(&new_temp, buf, sizeof(int)))
        return -EFAULT;
    atomic_set(&sensor.current_temp, new_temp);

    if (new_temp > atomic_read(&sensor.high_threshold) ||
        new_temp < atomic_read(&sensor.low_threshold)) {
        atomic_inc(&sensor.alert_count);
    }

    return sizeof(int);
}

static const struct file_operations temp_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = temp_ioctl,
    .write = temp_write,
};

static int __init temp_init(void)
{
    alloc_chrdev_region(&temp_dev, 0, 1, "TempSensor");
    cdev_init(&temp_cdev, &temp_fops);
    cdev_add(&temp_cdev, temp_dev, 1);
    temp_class = class_create(THIS_MODULE, "temp_class");
    temp_device = device_create(temp_class, NULL, temp_dev, NULL, "TempSensor");

    atomic_set(&sensor.current_temp, 25);
    atomic_set(&sensor.high_threshold, 30);
    atomic_set(&sensor.low_threshold, 20);
    atomic_set(&sensor.alert_count, 0);

    pr_info("TempSensor: initialized\n");
    return 0;
}

static void __exit temp_exit(void)
{
    device_destroy(temp_class, temp_dev);
    class_destroy(temp_class);
    cdev_del(&temp_cdev);
    unregister_chrdev_region(temp_dev, 1);
    pr_info("TempSensor: unloaded\n");
}

module_init(temp_init);
module_exit(temp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Temperature Sensor driver");