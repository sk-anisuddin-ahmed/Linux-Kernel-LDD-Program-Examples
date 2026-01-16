#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *thread1;
static struct task_struct *thread2;

static int thread_fn1(void *data)
{
    printk(KERN_INFO "Thread1 started (create + wake)\n");
    while (!kthread_should_stop()) 
	{
		printk(KERN_INFO "Thread1 Running\n");
        msleep(2000);
    }
    printk(KERN_INFO "Thread1 stopping\n");
    return 0;
}

static int thread_fn2(void *data)
{
    printk(KERN_INFO "Thread2 started (kthread_run)\n");
    while (!kthread_should_stop()) 
	{
        printk(KERN_INFO "Thread2 Running\n");
        ssleep(2);
    }
    printk(KERN_INFO "Thread2 stopping\n");
    return 0;
}

static int __init thread_init(void)
{
    printk(KERN_INFO "Threads Demo Module Init\n");

    thread1 = kthread_create(thread_fn1, NULL, "thread1_demo");
    if (!IS_ERR(thread1))
        wake_up_process(thread1);
	else
		thread1 = NULL;

    thread2 = kthread_run(thread_fn2, NULL, "thread2_demo");
    if (IS_ERR(thread2))
        thread2 = NULL;

    return 0;
}

static void __exit thread_exit(void)
{
    printk(KERN_INFO "Threads Demo Module Exit\n");

    if (thread1) 
	{
        kthread_stop(thread1);
        printk(KERN_INFO "Thread1 stopped\n");
    }
    if (thread2) 
	{
        kthread_stop(thread2);
        printk(KERN_INFO "Thread2 stopped\n");
    }
}

module_init(thread_init);
module_exit(thread_exit);

MODULE_LICENSE("GPL");
