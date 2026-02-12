#include <linux/interrupt.h>

static void my_softirq_action(struct softirq_action *action)
{
    printk(KERN_INFO "%s: executed!\n", __func__);
}

static int __init my_softirq_init(void)
{
    open_softirq(MY_SOFTIRQ, my_softirq_action);
    raise_softirq(MY_SOFTIRQ);
    return 0;
}

static void __exit my_softirq_exit(void)
{
    printk(KERN_INFO "%s: module unloaded.\n", __func__);
}

module_init(my_softirq_init);
module_exit(my_softirq_exit);

MODULE_LICENSE("GPL");