#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");

static unsigned long interrupt_count = 0;
static unsigned long tasklet_count = 0;
static unsigned long work_queue_count = 0;

static int irq_number = 0;

static irqreturn_t irq_handler(int irq, void *dev_id);
static void tasklet_func(unsigned long data);
static void work_func(struct work_struct *work);

DECLARE_TASKLET(demo_tasklet, tasklet_func, 0);
struct work_struct demo_work;

static irqreturn_t irq_handler(int irq, void *dev_id)
{
    interrupt_count++;
    tasklet_schedule(&demo_tasklet);
    return IRQ_HANDLED;
}

static void tasklet_func(unsigned long data)
{
    tasklet_count++;
}

static void work_func(struct work_struct *work)
{
    work_queue_count++;
}

static ssize_t irq_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "IRQ Count: %lu\nTasklet Count: %lu\nWork Queue Count: %lu\n",
                   interrupt_count, tasklet_count, work_queue_count);
}

static ssize_t irq_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if (irq_number > 0)
    {
        return sprintf(buf, "Registered IRQ: %d\nIRQ Handler: irq_handler\n", irq_number);
    }
    else
    {
        return sprintf(buf, "No IRQ registered\n");
    }
}

static ssize_t trigger_work_store(struct device *dev, struct device_attribute *attr,
                                  const char *buf, size_t count)
{
    schedule_work(&demo_work);
    return count;
}

static ssize_t trigger_tasklet_store(struct device *dev, struct device_attribute *attr,
                                     const char *buf, size_t count)
{
    tasklet_schedule(&demo_tasklet);
    return count;
}

static ssize_t reset_count_store(struct device *dev, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    interrupt_count = 0;
    tasklet_count = 0;
    work_queue_count = 0;
    return count;
}

static DEVICE_ATTR_RO(irq_count);
static DEVICE_ATTR_RO(irq_info);
static DEVICE_ATTR_WO(trigger_work);
static DEVICE_ATTR_WO(trigger_tasklet);
static DEVICE_ATTR_WO(reset_count);

static struct attribute *irq_attrs[] = {
    &dev_attr_irq_count.attr,
    &dev_attr_irq_info.attr,
    &dev_attr_trigger_work.attr,
    &dev_attr_trigger_tasklet.attr,
    &dev_attr_reset_count.attr,
    NULL};

static struct attribute_group irq_attr_group = {
    .attrs = irq_attrs,
};

static dev_t dev_number;
static struct cdev cdev_obj;
static struct class *irq_class;
static struct device *irq_device;

static ssize_t irq_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    char msg[256];
    int len;

    len = sprintf(msg,
                  "Interrupt Statistics:\n"
                  "IRQ Interrupts: %lu\n"
                  "Tasklet Executions: %lu\n"
                  "Work Queue Executions: %lu\n"
                  "Total Events: %lu\n",
                  interrupt_count, tasklet_count, work_queue_count,
                  interrupt_count + tasklet_count + work_queue_count);

    if (copy_to_user(buf, msg, len) != 0)
        return -EFAULT;

    return len;
}

static ssize_t irq_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    char command;

    if (get_user(command, buf) != 0)
        return -EFAULT;

    switch (command)
    {
    case 'T':
        tasklet_schedule(&demo_tasklet);
        break;
    case 'W':
        schedule_work(&demo_work);
        break;
    case 'R':
        interrupt_count = 0;
        tasklet_count = 0;
        work_queue_count = 0;
        break;
    }

    return size;
}

static const struct file_operations irq_fops = {
    .read = irq_read,
    .write = irq_write,
};

static int __init irq_init(void)
{
    int ret;

    INIT_WORK(&demo_work, work_func);

    ret = alloc_chrdev_region(&dev_number, 0, 1, "irq_demo");
    if (ret < 0)
        return ret;

    cdev_init(&cdev_obj, &irq_fops);
    ret = cdev_add(&cdev_obj, dev_number, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    irq_class = class_create(THIS_MODULE, "irq_demo_class");
    if (IS_ERR(irq_class))
    {
        cdev_del(&cdev_obj);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(irq_class);
    }

    irq_device = device_create(irq_class, NULL, dev_number, NULL, "irq_demo");
    if (IS_ERR(irq_device))
    {
        class_destroy(irq_class);
        cdev_del(&cdev_obj);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(irq_device);
    }

    ret = sysfs_create_group(&irq_device->kobj, &irq_attr_group);
    if (ret)
    {
        device_destroy(irq_class, dev_number);
        class_destroy(irq_class);
        cdev_del(&cdev_obj);
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    return 0;
}

static void __exit irq_exit(void)
{
    sysfs_remove_group(&irq_device->kobj, &irq_attr_group);
    device_destroy(irq_class, dev_number);
    class_destroy(irq_class);
    cdev_del(&cdev_obj);
    unregister_chrdev_region(dev_number, 1);
}

module_init(irq_init);
module_exit(irq_exit);