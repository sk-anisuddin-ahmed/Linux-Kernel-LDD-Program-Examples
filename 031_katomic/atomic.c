#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static atomic_t counter = ATOMIC_INIT(0);
static struct task_struct *thread1, *thread2;

static int inc_thread(void *data)
{
    while (!kthread_should_stop())
	{
        int val = atomic_inc_return(&counter);
        printk(KERN_INFO "Inc thread: counter = %d\n", val);
        msleep(500);
    }
    return 0;
}

static int dec_thread(void *data)
{
    while (!kthread_should_stop()) 
	{
        int val = atomic_dec_return(&counter);
        printk(KERN_INFO "Dec thread: counter = %d\n", val);
        msleep(700);
    }
    return 0;
}

static int __init atomic_init(void)
{
    printk(KERN_INFO "Atomic Demo Init\n");

    atomic_set(&counter, 0);

    thread1 = kthread_run(inc_thread, NULL, "inc_thread");
    thread2 = kthread_run(dec_thread, NULL, "dec_thread");

    return 0;
}

static void __exit atomic_exit(void)
{
    if (thread1) kthread_stop(thread1);
    if (thread2) kthread_stop(thread2);

    printk(KERN_INFO "Final counter value = %d\n", atomic_read(&counter));
    printk(KERN_INFO "Atomic Demo Exit\n");
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_LICENSE("GPL");
