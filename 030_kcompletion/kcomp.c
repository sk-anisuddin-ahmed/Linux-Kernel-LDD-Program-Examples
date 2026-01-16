#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct completion my_completion;
static struct task_struct *thread;

static int worker_fn(void *data)
{
    printk(KERN_INFO "Worker: work done, signaling completion\n");
    complete(&my_completion);
    return 0;
}

static int __init completion_init(void)
{
    printk(KERN_INFO "Completion Demo Init\n");
    init_completion(&my_completion);

    thread = kthread_run(worker_fn, NULL, "worker_thread");

    printk(KERN_INFO "Main: waiting for completion...\n");
    wait_for_completion(&my_completion);
    printk(KERN_INFO "Main: got completion signal!\n");

    return 0;
}

static void __exit completion_exit(void)
{
    printk(KERN_INFO "Completion Demo Exit\n");
}

module_init(completion_init);
module_exit(completion_exit);

MODULE_LICENSE("GPL");
