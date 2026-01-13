#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define DEVICE_CNT   2
#define BASE_MINOR   0
#define DEVICE_NAME  "dual_poll"
#define CLASS_NAME   "dual_class"
#define DEVICE_PARENT NULL
#define DEVICE_DATA   NULL
#define BUF_SIZE     128

static dev_t devt;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_devices[DEVICE_CNT];

static wait_queue_head_t wq_array[DEVICE_CNT];
static int event_flag[DEVICE_CNT] = {0};
static char buffer[DEVICE_CNT][BUF_SIZE];
static size_t buffer_size[DEVICE_CNT] = {0};

#define print(fmt, ...) pr_info(fmt "\n", ##__VA_ARGS__)

static unsigned int my_poll(struct file *file, struct poll_table_struct *wait)
{
    int minor = iminor(file->f_inode);
    unsigned int mask = 0;

    poll_wait(file, &wq_array[minor], wait);

    if (event_flag[minor])
        mask = POLLIN | POLLRDNORM;

    return mask;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *pos)
{
    int minor = iminor(file->f_inode);
    ssize_t ret;

    /* Block until event_flag is set */
    if (!event_flag[minor]) {
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        wait_event_interruptible(wq_array[minor], event_flag[minor]);
    }

    ret = simple_read_from_buffer(buf, len, pos,
                                  buffer[minor], buffer_size[minor]);

    /* Clear event once fully read */
    if (*pos >= buffer_size[minor]) {
        event_flag[minor] = 0;
        buffer_size[minor] = 0;
    }

    return ret;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    int minor = iminor(file->f_inode);

    if (len > BUF_SIZE)
        len = BUF_SIZE;

    if (copy_from_user(buffer[minor], buf, len))
        return -EFAULT;

    buffer_size[minor] = len;
    *pos = 0;
    event_flag[minor] = 1;

    wake_up_interruptible(&wq_array[minor]);
    print("Device %d event triggered", minor);

    return len;
}

static const struct file_operations my_fops = {
    .owner   = THIS_MODULE,
    .read    = my_read,
    .write   = my_write,
    .poll    = my_poll,
    .llseek  = default_llseek,
};

static int __init dev_init(void)
{
    int ret, i;
    char dev_name[32];

    ret = alloc_chrdev_region(&devt, BASE_MINOR, DEVICE_CNT, DEVICE_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, devt, DEVICE_CNT);
    if (ret < 0) {
        unregister_chrdev_region(devt, DEVICE_CNT);
        return ret;
    }

    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(devt, DEVICE_CNT);
        return PTR_ERR(my_class);
    }

    for (i = 0; i < DEVICE_CNT; i++) {
        init_waitqueue_head(&wq_array[i]);
        snprintf(dev_name, sizeof(dev_name), "%s%d", DEVICE_NAME, i);

        my_devices[i] = device_create(my_class, DEVICE_PARENT,
                                      MKDEV(MAJOR(devt), i),
                                      DEVICE_DATA, dev_name);
        if (IS_ERR(my_devices[i])) {
            while (--i >= 0)
                device_destroy(my_class, MKDEV(MAJOR(devt), i));
            class_destroy(my_class);
            cdev_del(&my_cdev);
            unregister_chrdev_region(devt, DEVICE_CNT);
            return PTR_ERR(my_devices[i]);
        }
    }

    print("module loaded (major=%d)", MAJOR(devt));
    return 0;
}

static void __exit dev_exit(void)
{
    int i;

    for (i = 0; i < DEVICE_CNT; i++)
        device_destroy(my_class, MKDEV(MAJOR(devt), i));

    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(devt, DEVICE_CNT);

    print("module unloaded");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");

module_init(dev_init);
module_exit(dev_exit);
