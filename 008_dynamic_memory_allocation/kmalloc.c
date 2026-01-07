#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>

MODULE_LICENSE("GPL");

struct device_config
{
    uint32_t device_id;
    uint16_t port;
    uint8_t irq;
    char name[32];
};

static int kmalloc_init(void)
{
    pr_info("kmalloc module loading\n");

    phys_addr_t phys_addr;
    struct device_config *config;
    void *buffer;
    size_t actual_size;

    pr_info("\nkmalloc with GFP_KERNEL\n");
    buffer = kmalloc(1024, GFP_KERNEL);
    if (!buffer)
    {
        pr_err("kmalloc failed\n");
        return -ENOMEM;
    }

    pr_info("\tVirtual address: %px\n", buffer);
    phys_addr = virt_to_phys(buffer);
    pr_info("\tPhysical address: 0x%pa\n", &phys_addr);
    pr_info("\tDifference (PAGE_OFFSET): 0x%lx\n", (unsigned long)buffer - (unsigned long)phys_addr);
    actual_size = ksize(buffer);
    pr_info("\tRequested: 1024 bytes, Allocated: %zu bytes\n", actual_size);
    kfree(buffer);
    pr_info("\tFreed memory\n");

    pr_info("\nAllocate memory to struct\n");
    config = kmalloc(sizeof(struct device_config), GFP_KERNEL);
    if (!config)
    {
        pr_err("kmalloc for config failed\n");
        return -ENOMEM;
    }

    pr_info("\tConfig virtual: %px\n", config);
    pr_info("\tStruct size: %zu bytes\n", sizeof(struct device_config));
    kfree(config);
    pr_info("\tFreed config structure\n");

    /* Example 3: kzalloc - zero-initialized allocation */
    pr_info("\nkzalloc zero-initialized allocation\n");
    buffer = kzalloc(256, GFP_KERNEL);
    if (!buffer)
    {
        pr_err("kzalloc failed\n");
        return -ENOMEM;
    }

    pr_info("\tkzalloc allocated 256 bytes (all zeros)\n");
    pr_info("\tFirst 4 bytes: 0x%02x%02x%02x%02x\n",
            *(uint8_t *)buffer, *(uint8_t *)(buffer + 1),
            *(uint8_t *)(buffer + 2), *(uint8_t *)(buffer + 3));

    kfree(buffer);
    pr_info("\tFreed kzalloc'd memory\n");

    pr_info("\nkmalloc with GFP_ATOMIC\n");

    buffer = kmalloc(512, GFP_ATOMIC);
    if (buffer)
    {
        pr_info("\tGFP_ATOMIC: allocated 512 bytes\n");
        kfree(buffer);
    }
    else
    {
        pr_info("\tGFP_ATOMIC: allocation failed\n");
    }

    pr_info("\nkmalloc(0)\n");
    buffer = kmalloc(0, GFP_KERNEL);
    if (buffer)
    {
        pr_info("\tkmalloc(0) returned: %px\n", buffer);
        kfree(buffer);
    }

    return 0;
}

static void kmalloc_exit(void)
{
    pr_info("kmalloc module unloading\n");
}

module_init(kmalloc_init);
module_exit(kmalloc_exit);