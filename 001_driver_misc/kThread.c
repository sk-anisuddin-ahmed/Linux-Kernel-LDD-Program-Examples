#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/mutex.h>

static struct task_struct *th1;
static struct task_struct *th2;

static int counter;
static DEFINE_MUTEX(lock);

static int worker(void *data)
{
    const char *name = data;

    while (!kthread_should_stop()) 
	{
        mutex_lock(&lock);
        counter++;
        mutex_unlock(&lock);

        pr_info("thread=%s pid=%d cpu=%d counter=%d\n",
                name, current->pid,
                smp_processor_id(),
                counter);
        msleep(1000);
    }

    pr_info("thread=%s stopping\n", name);
    return 0;
}

static int __init kThread_init(void)
{
    th1 = kthread_create(worker, "T1", "kth_T1");
    wake_up_process(th1);

    th2 = kthread_run(worker, "T2", "kth_T2");

    pr_info("kThread loaded\n");
    return 0;
}

static void __exit kThread_exit(void)
{
    if (th1)
        kthread_stop(th1);

    if (th2)
        kthread_stop(th2);

    pr_info("kThread unloaded\n");
}

module_init(kThread_init);
module_exit(kThread_exit);
MODULE_LICENSE("GPL");
