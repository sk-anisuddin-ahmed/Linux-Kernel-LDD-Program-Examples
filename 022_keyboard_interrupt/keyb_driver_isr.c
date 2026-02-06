#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <asm/io.h>

#define KEYBOARD_IRQ	0x1

static struct work_struct kb_work;

static void kb_work_fn(struct work_struct *work)
{
    printk(KERN_INFO "Keyboard IRQ Hit\n");
}

static irqreturn_t keyboard_irq_handler(int irq, void *dev_id)
{
    schedule_work(&kb_work);
    return IRQ_NONE;
}

static int __init keyboard_driver_init(void)
{
    INIT_WORK(&kb_work, kb_work_fn);

    if (request_irq(KEYBOARD_IRQ,
                    keyboard_irq_handler,
                    IRQF_SHARED,
                    "keyb_snif",
                    NULL)) 
	{
        printk(KERN_ERR "Failed to register keyboard IRQ\n");
        return -1;
    }

    printk(KERN_INFO "Keyboard sniffer loaded\n");
    return 0;
}

static void __exit keyboard_driver_exit(void)
{
    free_irq(KEYBOARD_IRQ, (void *)(keyboard_irq_handler));
    flush_scheduled_work(&kb_work);
    printk(KERN_INFO "Keyboard sniffer unloaded\n");
}

module_init(keyboard_driver_init);
module_exit(keyboard_driver_exit);

MODULE_LICENSE("GPL");