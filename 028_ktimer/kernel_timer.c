#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");

static struct timer_list my_timer;
static unsigned int timer_count = 0;

static void timer_callback(struct timer_list *timer)
{
    timer_count++;
    printk(KERN_INFO "kernel_timer: Timer Callback Called [%d]\n", timer_count);

    mod_timer(&my_timer, jiffies + (2 * HZ));
}

static int __init kernel_timer_init(void)
{
    printk(KERN_INFO "kernel_timer: Initializing Timer Driver\n");

    timer_setup(&my_timer, timer_callback, 0);

    my_timer.expires = jiffies + (2 * HZ);

    add_timer(&my_timer);

    printk(KERN_INFO "kernel_timer: Timer Started - expires in 2 seconds\n");

    return 0;
}

static void __exit kernel_timer_cleanup(void)
{
    printk(KERN_INFO "kernel_timer: Cleaning Up Timer Driver\n");

    del_timer(&my_timer);

    printk(KERN_INFO "kernel_timer: Timer Removed\n");
}

module_init(kernel_timer_init);
module_exit(kernel_timer_cleanup);
