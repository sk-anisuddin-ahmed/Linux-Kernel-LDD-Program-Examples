#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/irq.h>

#define IRQ_NO 11

static struct work_struct workqueue;

static int etx_value = 0;
static struct kobject *kobj_ref;

static void workqueue_fn(struct work_struct *work)
{
    printk(KERN_INFO "Executing Workqueue Function\n");
}

static irqreturn_t irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "IRQ %d: Interrupt Occurred\n", irq);
    schedule_work(&workqueue);
    return IRQ_HANDLED;
}

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d", etx_value);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &etx_value);
    printk(KERN_INFO "Sysfs - Write: %d\n", etx_value);

    asm("int $0x3B");

    return count;
}

/* Sysfs attribute */
struct kobj_attribute etx_attr = __ATTR(etx_value, 0660, sysfs_show, sysfs_store);

/* Module Init */
static int __init etx_driver_init(void)
{
    int ret;

    /* Register IRQ */
    ret = request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", (void *)(irq_handler));
    if (ret) 
	{
        printk(KERN_ERR "Cannot register IRQ %d\n", IRQ_NO);
        return ret;
    }

    /* Init workqueue */
    INIT_WORK(&workqueue, workqueue_fn);

    /* Create sysfs entry */
    kobj_ref = kobject_create_and_add("etx_sysfs", kernel_kobj);
    if (sysfs_create_file(kobj_ref, &etx_attr.attr)) 
	{
        printk(KERN_ERR "Cannot create sysfs file\n");
        kobject_put(kobj_ref);
        free_irq(IRQ_NO, (void *)(irq_handler));
        return -1;
    }

    printk(KERN_INFO "Driver loaded, IRQ %d registered\n", IRQ_NO);
    return 0;
}

static void __exit etx_driver_exit(void)
{
    free_irq(IRQ_NO, (void *)(irq_handler));
    sysfs_remove_file(kobj_ref, &etx_attr.attr);
    kobject_put(kobj_ref);
    printk(KERN_INFO "Driver unloaded\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");