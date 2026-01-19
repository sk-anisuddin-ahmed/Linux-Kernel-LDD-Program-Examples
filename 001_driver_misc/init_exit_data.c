#include <linux/module.h>
#include <linux/kernel.h>

static int __initdata init_val = 10;

static int __exitdata exit_val = 20;

static int __init my_init(void)
{
    pr_info("init: init_val=%d\n", init_val);
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("exit: exit_val=%d\n", exit_val);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
