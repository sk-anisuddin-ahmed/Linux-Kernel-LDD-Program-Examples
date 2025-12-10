#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>

#define LED_ON             _IO('A', 0)
#define LED_OFF            _IO('B', 1)
#define LED_SET_BRIGHTNESS _IOW('C', 2, int)
#define LED_GET_STATE      _IOR('D', 3, int)
#define LED_GET_BRIGHTNESS _IOR('E', 4, int)

typedef struct __attribute__((packed)) {
    _Bool state;
    uint8_t brightness;
    uint8_t temperature;
} lightStatus;

static atomic_t g_state        = ATOMIC_INIT(0);
static atomic_t g_brightness   = ATOMIC_INIT(0);
static atomic_t g_temperature  = ATOMIC_INIT(25);

static dev_t light_dev_number;
static struct cdev light_cdev;
static struct class *light_class;
static struct device *light_device;

static ssize_t light_read(struct file *file,
                          char __user *user_buffer,
                          size_t length,
                          loff_t *offset)
{
    lightStatus lRead;

    lRead.state       = atomic_read(&g_state);
    lRead.brightness  = (uint8_t)atomic_read(&g_brightness);
    lRead.temperature = (uint8_t)atomic_read(&g_temperature);

    if (copy_to_user(user_buffer, &lRead, sizeof(lRead)))
        return -EFAULT;

    return sizeof(lRead);
}

static ssize_t light_write(struct file *file,
                           const char __user *user_buffer,
                           size_t length,
                           loff_t *offset)
{
    lightStatus lWrite;

    if (copy_from_user(&lWrite, user_buffer, sizeof(lWrite)))
        return -EFAULT;

    atomic_set(&g_state, lWrite.state);
    atomic_set(&g_brightness, lWrite.brightness);
    atomic_set(&g_temperature, lWrite.temperature);

    return sizeof(lWrite);
}

static int light_open(struct inode *inode, struct file *file)
{
    pr_info("Light Device: opened\n");
    return 0;
}

static int light_release(struct inode *inode, struct file *file)
{
    pr_info("Light Device: closed\n");
    return 0;
}

static long light_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int val;

    switch (cmd) {
    case LED_ON:
        atomic_set(&g_state, 1);
        pr_info("LightDevice: LED ON\n");
        return 0;

    case LED_OFF:
        atomic_set(&g_state, 0);
        pr_info("LightDevice: LED OFF\n");
        return 0;

    case LED_SET_BRIGHTNESS:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;
        atomic_set(&g_brightness, val);
        pr_info("LightDevice: brightness = %d\n", atomic_read(&g_brightness));
        return 0;

    case LED_GET_STATE:
        val = atomic_read(&g_state);
        if (copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        return 0;

    case LED_GET_BRIGHTNESS:
        val = atomic_read(&g_brightness);
        if (copy_to_user((int __user *)arg, &val, sizeof(int)))
            return -EFAULT;
        return 0;

    default:
        return -EINVAL;
    }
}

static const struct file_operations light_fops = {
    .owner          = THIS_MODULE,
    .read           = light_read,
    .write          = light_write,
    .open           = light_open,
    .release        = light_release,
    .unlocked_ioctl = light_ioctl
};

static int __init light_driver_init(void)
{
    alloc_chrdev_region(&light_dev_number, 0, 1, "LightDevice");
    cdev_init(&light_cdev, &light_fops);
    light_cdev.owner = THIS_MODULE;
    cdev_add(&light_cdev, light_dev_number, 1);
    light_class = class_create(THIS_MODULE, "light_class");
    light_device = device_create(light_class, NULL, light_dev_number, NULL, "LightDevice");

    pr_info("Light Device: driver initialized (major=%d minor=%d)\n",
            MAJOR(light_dev_number), MINOR(light_dev_number));
    return 0;
}

static void __exit light_driver_exit(void)
{
    device_destroy(light_class, light_dev_number);
    class_destroy(light_class);
    cdev_del(&light_cdev);
    unregister_chrdev_region(light_dev_number, 1);
    pr_info("Light Device: unloaded\n");
}

module_init(light_driver_init);
module_exit(light_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Light Control Operation");
