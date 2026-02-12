#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

static struct tasklet_struct my_tasklet;

static void demo_tasklet_fn(unsigned long data)
{
    printk("%s: running in %s context\n", __func__ , current->comm);
}

static int __init softirq_init(void)
{
    pr_info("%s: executed\n", __func__);

    tasklet_init(&my_tasklet, demo_tasklet_fn, 0);
    tasklet_schedule(&my_tasklet);

    return 0;
}

static void __exit softirq_exit(void)
{
    tasklet_kill(&my_tasklet);
    pr_info("%s: executed\n", __func__);
}

module_init(softirq_init);
module_exit(softirq_exit);

MODULE_LICENSE("GPL");