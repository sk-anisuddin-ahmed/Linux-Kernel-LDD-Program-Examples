#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#define MYDEV_NAME       "mydev"
#define MYDEV_CLASS_NAME "mydev_class"
#define MYDEV_COUNT      2
#define BUF_SIZE         1024

struct mydev_device {
    struct cdev cdev;        /* character device handle */
    struct device *dev;      /* sysfs device object */
    struct mutex lock;       /* protects buffer */
    char buf[BUF_SIZE];      /* simple fixed buffer */
    size_t len;              /* current data length */
};

static dev_t mydev_devt;
static struct class *mydev_class;
static struct mydev_device *mydevs;

/* ---------- File operations ---------- */

static int mydev_open(struct inode *inode, struct file *filp)
{
    struct mydev_device *d = container_of(inode->i_cdev, struct mydev_device, cdev);
    filp->private_data = d;
    return 0;
}

static int mydev_release(struct inode *inode, struct file *filp)
{
    filp->private_data = NULL;
    return 0;
}

static ssize_t mydev_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
    struct mydev_device *d = filp->private_data;
    ssize_t ret;

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    if (*ppos >= d->len) {
        ret = 0; /* EOF */
        goto out;
    }

    if (count > d->len - *ppos)
        count = d->len - *ppos;

    if (copy_to_user(ubuf, d->buf + *ppos, count)) {
        ret = -EFAULT;
        goto out;
    }

    *ppos += count;
    ret = count;

out:
    mutex_unlock(&d->lock);
    return ret;
}

static ssize_t mydev_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct mydev_device *d = filp->private_data;
    ssize_t ret;

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    if (*ppos >= BUF_SIZE) {
        ret = -ENOSPC;
        goto out;
    }

    if (count > BUF_SIZE - *ppos)
        count = BUF_SIZE - *ppos;

    if (copy_from_user(d->buf + *ppos, ubuf, count)) {
        ret = -EFAULT;
        goto out;
    }

    *ppos += count;
    if (d->len < *ppos)
        d->len = *ppos;

    ret = count;

out:
    mutex_unlock(&d->lock);
    return ret;
}

static loff_t mydev_llseek(struct file *filp, loff_t off, int whence)
{
    struct mydev_device *d = filp->private_data;
    loff_t newpos;

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    switch (whence) {
    case SEEK_SET: newpos = off; break;
    case SEEK_CUR: newpos = filp->f_pos + off; break;
    case SEEK_END: newpos = d->len + off; break;
    default:
        mutex_unlock(&d->lock);
        return -EINVAL;
    }

    if (newpos < 0 || newpos > BUF_SIZE) {
        mutex_unlock(&d->lock);
        return -EINVAL;
    }

    filp->f_pos = newpos;
    mutex_unlock(&d->lock);
    return newpos;
}

static const struct file_operations mydev_fops = {
    .owner   = THIS_MODULE,
    .open    = mydev_open,
    .release = mydev_release,
    .read    = mydev_read,
    .write   = mydev_write,
    .llseek  = mydev_llseek,
};

/* ---------- Sysfs attributes (using sysfs_emit) ---------- */

static ssize_t data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct mydev_device *d = dev_get_drvdata(dev);
    ssize_t out;

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    out = sysfs_emit(buf, "%.*s\n", (int)d->len, d->buf);

    mutex_unlock(&d->lock);
    return out;
}

static ssize_t data_store(struct device *dev, struct device_attribute *attr,
                          const char *buf, size_t count)
{
    struct mydev_device *d = dev_get_drvdata(dev);

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    if (count > BUF_SIZE)
        count = BUF_SIZE;

    memcpy(d->buf, buf, count);
    d->len = count;

    mutex_unlock(&d->lock);
    return count;
}

static ssize_t stats_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct mydev_device *d = dev_get_drvdata(dev);
    ssize_t out;

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    out = sysfs_emit(buf, "len=%zu\n", d->len);

    mutex_unlock(&d->lock);
    return out;
}

static ssize_t reset_store(struct device *dev, struct device_attribute *attr,
                           const char *buf, size_t count)
{
    struct mydev_device *d = dev_get_drvdata(dev);

    if (!d)
        return -EINVAL;

    if (mutex_lock_interruptible(&d->lock))
        return -ERESTARTSYS;

    d->len = 0;
    memset(d->buf, 0, BUF_SIZE);

    mutex_unlock(&d->lock);
    return count;
}

static DEVICE_ATTR_RW(data);
static DEVICE_ATTR_RO(stats);
static DEVICE_ATTR_WO(reset);

/* ---------- Module init/exit ---------- */

static int __init mydev_init(void)
{
    int ret, i;

    ret = alloc_chrdev_region(&mydev_devt, 0, MYDEV_COUNT, MYDEV_NAME);
    if (ret)
        return ret;

    mydev_class = class_create(THIS_MODULE, MYDEV_CLASS_NAME);
    if (IS_ERR(mydev_class)) {
        ret = PTR_ERR(mydev_class);
        unregister_chrdev_region(mydev_devt, MYDEV_COUNT);
        return ret;
    }

    mydevs = kcalloc(MYDEV_COUNT, sizeof(*mydevs), GFP_KERNEL);
    if (!mydevs) {
        ret = -ENOMEM;
        class_destroy(mydev_class);
        unregister_chrdev_region(mydev_devt, MYDEV_COUNT);
        return ret;
    }

    for (i = 0; i < MYDEV_COUNT; i++) {
        dev_t devno = MKDEV(MAJOR(mydev_devt), MINOR(mydev_devt) + i);

        mutex_init(&mydevs[i].lock);
        mydevs[i].len = 0;
        memset(mydevs[i].buf, 0, BUF_SIZE);

        cdev_init(&mydevs[i].cdev, &mydev_fops);
        mydevs[i].cdev.owner = THIS_MODULE;

        ret = cdev_add(&mydevs[i].cdev, devno, 1);
        if (ret)
            goto err_loop;

        mydevs[i].dev = device_create(mydev_class, NULL, devno, NULL, "%s%d", MYDEV_NAME, i);
        if (IS_ERR(mydevs[i].dev)) {
            ret = PTR_ERR(mydevs[i].dev);
            cdev_del(&mydevs[i].cdev);
            goto err_loop;
        }

        dev_set_drvdata(mydevs[i].dev, &mydevs[i]);

        ret = device_create_file(mydevs[i].dev, &dev_attr_data);
        if (ret)
            goto err_attr;

        ret = device_create_file(mydevs[i].dev, &dev_attr_stats);
        if (ret)
            goto err_attr_stats;

        ret = device_create_file(mydevs[i].dev, &dev_attr_reset);
        if (ret)
            goto err_attr_reset;

        continue;

err_attr_reset:
        device_remove_file(mydevs[i].dev, &dev_attr_stats);
err_attr_stats:
        device_remove_file(mydevs[i].dev, &dev_attr_data);
err_attr:
        device_destroy(mydev_class, devno);
        cdev_del(&mydevs[i].cdev);
        ret = ret ? ret : -EINVAL;
        goto err_loop;
    }

    pr_info("mydev: loaded %d devices (major=%d)\n", MYDEV_COUNT, MAJOR(mydev_devt));
    return 0;

err_loop:
    /* Roll back any devices already created */
    while (--i >= 0) {
        dev_t devno = MKDEV(MAJOR(mydev_devt), MINOR(mydev_devt) + i);
        if (!IS_ERR_OR_NULL(mydevs[i].dev)) {
            device_remove_file(mydevs[i].dev, &dev_attr_reset);
            device_remove_file(mydevs[i].dev, &dev_attr_stats);
            device_remove_file(mydevs[i].dev, &dev_attr_data);
            device_destroy(mydev_class, devno);
        }
        cdev_del(&mydevs[i].cdev);
    }
    kfree(mydevs);
    class_destroy(mydev_class);
    unregister_chrdev_region(mydev_devt, MYDEV_COUNT);
    return ret;
}

static void __exit mydev_exit(void)
{
    int i;

    for (i = 0; i < MYDEV_COUNT; i++) {
        dev_t devno = MKDEV(MAJOR(mydev_devt), MINOR(mydev_devt) + i);

        device_remove_file(mydevs[i].dev, &dev_attr_reset);
        device_remove_file(mydevs[i].dev, &dev_attr_stats);
        device_remove_file(mydevs[i].dev, &dev_attr_data);

        device_destroy(mydev_class, devno);
        cdev_del(&mydevs[i].cdev);
    }

    kfree(mydevs);
    class_destroy(mydev_class);
    unregister_chrdev_region(mydev_devt, MYDEV_COUNT);

    pr_info("mydev: unloaded\n");
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sk");
MODULE_DESCRIPTION("Simple multi-minor char driver with per-device sysfs attributes");
