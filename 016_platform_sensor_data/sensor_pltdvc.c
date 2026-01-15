#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#define PLATFORM_DRIVER_NAME "sensor_driver"
#define DEVICE_ID -1 /* Auto-assign platform device ID */

/* Memory resource for sensor device */
static struct resource sensor_resources[] = {
	{
		.name = "sensor_mem",
		.start = 0x50000000,
		.end = 0x50000FFF,
		.flags = IORESOURCE_MEM,
	},
};

/* Platform device structure */
static struct platform_device sensor_pltdvc = {
	.name = PLATFORM_DRIVER_NAME,
	.id = DEVICE_ID,
	.resource = sensor_resources,
	.num_resources = ARRAY_SIZE(sensor_resources),
};

static int __init sensor_dvc_init(void)
{
	platform_device_register(&sensor_pltdvc);
	pr_info("Sensor platform device registered\n");
	return 0;
}

static void __exit sensor_dvc_exit(void)
{
	platform_device_unregister(&sensor_pltdvc);
	pr_info("Sensor platform device unregistered\n");
}

MODULE_LICENSE("GPL");
module_init(sensor_dvc_init);
module_exit(sensor_dvc_exit);