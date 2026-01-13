#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Driver Module");

static int my_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Probe called for device: %s\n", pdev->name);
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Remove called for device: %s\n", pdev->name);
    return 0;
}

static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "myplatformdev",
        .owner = THIS_MODULE,
    },
    .probe = my_probe,
    .remove = my_remove,
};

static int __init pdrv_init(void)
{
    printk(KERN_INFO "Platform driver loaded\n");
    return platform_driver_register(&my_platform_driver);
}

static void __exit pdrv_exit(void)
{
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "Platform driver unloaded\n");
}

module_init(pdrv_init);
module_exit(pdrv_exit);