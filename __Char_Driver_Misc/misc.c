#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define DEVICE_NAME "my_misc"

static char kernel_buffer[256] = "Hello India\n";

static ssize_t misc_read(struct file *file,
                         char __user *buf,
                         size_t count,
                         loff_t *ppos)
{
    if (!access_ok(buf, count))
        return -EFAULT;

    return simple_read_from_buffer(buf,
                                   count,
                                   ppos,
                                   kernel_buffer,
                                   strlen(kernel_buffer));
}

static ssize_t misc_write(struct file *file,
                          const char __user *buf,
                          size_t count,
                          loff_t *ppos)
{
    if (count > sizeof(kernel_buffer) - 1)
        return -EINVAL;

    if (!access_ok(buf, count))
        return -EFAULT;

    if (copy_from_user(kernel_buffer, buf, count))
        return -EFAULT;

    kernel_buffer[count] = '\0';
    return count;
}

static const struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .read  = misc_read,
    .write = misc_write,
};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,   /* auto minor */
    .name  = DEVICE_NAME,
    .fops  = &misc_fops,
    .mode  = 0666,                 /* rw-rw-rw- */
};

static int __init misc_init(void)
{
    int ret;

    ret = misc_register(&misc_dev);
    if (ret) {
        pr_err("Failed to register misc device\n");
        return ret;
    }

    pr_info("Misc device /dev/%s registered\n", DEVICE_NAME);
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&misc_dev);
    pr_info("Misc device unregistered\n");
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");
MODULE_DESCRIPTION("Misc device driver with access_ok usage");
