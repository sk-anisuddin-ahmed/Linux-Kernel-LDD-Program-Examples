#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

static void my_tasklet_func(struct tasklet_struct *t)
{
    pr_info("%s: executed\n", __func__);
    pr_info("%s: in_irq=%d, in_softirq=%d, in_interrupt=%d, in_task=%d\n",
            __func__, !!in_irq(), !!in_softirq(), !!in_interrupt(), !!in_task());
}

DECLARE_TASKLET(my_tasklet, my_tasklet_func);

static int __init my_tasklet_init(void)
{
    pr_info("%s: loaded\n", __func__);
    tasklet_schedule(&my_tasklet);
    return 0;
}

static void __exit my_tasklet_exit(void)
{
    tasklet_kill(&my_tasklet);
    pr_info("%s: removed\n", __func__);
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);

MODULE_LICENSE("GPL");