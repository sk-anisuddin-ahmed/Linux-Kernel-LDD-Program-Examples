#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");

static struct hrtimer hr_timer;

static enum hrtimer_restart hrt_timer_callback(struct hrtimer *timer)
{
    printk(KERN_INFO "HRT: Timer callback executed\n");

    hrtimer_forward_now(timer, ktime_set(0, 500000000)); /* 500ms in nanoseconds */

    return HRTIMER_RESTART;
}

static int __init hrt_timer_init(void)
{
    ktime_t ktime;

    printk(KERN_INFO "HRT: Initializing High-Resolution Timer module\n");

    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    hr_timer.function = hrt_timer_callback;

    ktime = ktime_set(1, 0); /* 1 second */

    hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
    printk(KERN_INFO "HRT: Timer started - will fire every 500ms after 1s delay\n");

    return 0;
}

static void __exit hrt_timer_exit(void)
{
    printk(KERN_INFO "HRT: Cleaning up High-Resolution Timer module\n");

    int ret = hrtimer_cancel(&hr_timer);

    if (ret == 0)
        printk(KERN_INFO "HRT: Timer was not active\n");
    else
        printk(KERN_INFO "HRT: Timer was active and has been cancelled\n");

    printk(KERN_INFO "HRT: Module unloaded\n");
}

module_init(hrt_timer_init);
module_exit(hrt_timer_exit);
