#include <linux/modules.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#define PLATFORM_DRIVER_NAME	"sensor_driver"

static int sensor_probe (struct platform_device* pltdvc)
{
	pr_info("sensor probe function called\n");
}

static void sensor_remove (struct platform_device* pltdvc)
{
	pr_info("sensor remove function called\n");
}

struct platform_driver sensor_pltdrv = {
	.driver = {
		.name = PLATFORM_DRIVER_NAME,
		.owner = THIS_MODULE
	}
	.probe = sensor_probe,
	.remove = sensor_remove
};

static int __init sensor_init(void)
{
	pr_info("sensor module loaded\n");
	return 0;
}

static void __exit sensor_exit(void)
{
	pr_info("sensor module unloaded\n");
}

MODULE__LICENSE("GPL");
module_init(sensor_init);
module_exit(sensor_exit);