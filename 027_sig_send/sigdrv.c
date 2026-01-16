#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/sched/signal.h>
#include <string.h>

#define SIGETX 44
#define REG_CURRENT_TASK _IOW('a', 'a', int32_t *)
#define IRQ_NO 11

static struct task_struct *task = NULL;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

/* Interrupt handler */
static irqreturn_t irq_handler(int irq, void *dev_id)
{
    struct siginfo info;
    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = 1;

    if (task != NULL)
    {
        printk(KERN_INFO "Sending signal to app\n");
        if (send_sig_info(SIGETX, &info, task) < 0)
            printk(KERN_INFO "Unable to send signal\n");
    }
    return IRQ_HANDLED;
}

/* IOCTL to register current task */
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == REG_CURRENT_TASK)
    {
        task = get_current();
        printk(KERN_INFO "Registered current task for signals\n");
    }
    return 0;
}

/* Release: unregister task */
static int etx_release(struct inode *inode, struct file *file)
{
    if (task == get_current())
        task = NULL;
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = etx_ioctl,
    .release = etx_release,
};

static int __init etx_driver_init(void)
{
    alloc_chrdev_region(&dev, 0, 1, "etx_dev");
    cdev_init(&etx_cdev, &fops);
    cdev_add(&etx_cdev, dev, 1);
    dev_class = class_create(THIS_MODULE, "etx_class");
    device_create(dev_class, NULL, dev, NULL, "etx_device");

    request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", (void *)irq_handler);
    printk(KERN_INFO "Driver loaded\n");
    return 0;
}

static void __exit etx_driver_exit(void)
{
    free_irq(IRQ_NO, (void *)irq_handler);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Driver unloaded\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
MODULE_LICENSE("GPL");
