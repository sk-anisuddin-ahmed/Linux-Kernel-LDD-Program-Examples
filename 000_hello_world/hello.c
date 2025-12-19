#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timekeeping.h>

static int __init hello_init(void) 
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);

    printk(KERN_INFO "Module loaded at %lld seconds since epoch\n", ts.tv_sec);
    printk(KERN_INFO "Hello, Sk Anisuddin Ahmed!\n");

    return 0;
}

static void __exit hello_exit(void) 
{
    printk(KERN_INFO "Goodbye, Module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sk Anisuddin Ahmed");
MODULE_DESCRIPTION("Hello World Kernel Module");
