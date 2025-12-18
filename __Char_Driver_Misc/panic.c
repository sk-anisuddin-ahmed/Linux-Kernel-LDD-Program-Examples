#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init panic_init(void)
{
    pr_emerg("kernel panic now\n");

    panic("simulated kernel panic");

    return 0;
}

static void __exit panic_exit(void)
{
    pr_info("panic exit\n");
}

module_init(panic_init);
module_exit(panic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");
MODULE_DESCRIPTION("Simulated kernel panic");
