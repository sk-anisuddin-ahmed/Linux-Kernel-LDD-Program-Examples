#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

static int vmalloc_init(void)
{
    pr_info("vmalloc loading\n");

    void *large_buffer;
    
    pr_info("\nvmalloc\n");
    large_buffer = vmalloc(1024 * 1024);
    if (!large_buffer)
    {
        pr_err("vmalloc failed\n");
        return -ENOMEM;
    }

    pr_info("\tVirtual address: %px\n", large_buffer);
    vfree(large_buffer);
    pr_info("\tFreed with vfree()\n");

    pr_info("\nLarge allocation (10 MB) - beyond kmalloc limit (4 MB)\n");

    large_buffer = vmalloc(10 * 1024 * 1024);
    if (!large_buffer)
    {
        pr_err("vmalloc(10 MB) failed - out of memory?\n");
        return -ENOMEM;
    }

    pr_info("\tSuccessfully allocated 10 MB\n");

    vfree(large_buffer);
    pr_info("\tFreed 10 MB\n");

    return 0;
}

static void test_vmalloc_exit(void)
{
    pr_info("vmalloc unloading\n");
}

module_init(test_vmalloc_init);
module_exit(test_vmalloc_exit);
