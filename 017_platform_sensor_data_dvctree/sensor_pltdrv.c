#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#define PLATFORM_DRIVER_NAME "sensor_driver"

struct sensor_dev
{
	struct resource *mem_res;
	void __iomem *base_addr;
	unsigned int control_value;
};

/* Sysfs show: read temperature register */
static ssize_t temp_show(struct device *dev,
						 struct device_attribute *attr,
						 char *buf)
{
	struct sensor_dev *pdata = dev_get_drvdata(dev);
	unsigned int temp_value = ioread32(pdata->base_addr + 0x00);
	return sprintf(buf, "%u\n", temp_value);
}

/* Sysfs store: write control register */
static ssize_t control_store(struct device *dev,
							 struct device_attribute *attr,
							 const char *buf,
							 size_t count)
{
	struct sensor_dev *pdata = dev_get_drvdata(dev);
	unsigned int val;

	if (kstrtouint(buf, 0, &val) == 0)
	{
		pdata->control_value = val;
		iowrite32(val, pdata->base_addr + 0x04);
		pr_info("Control register updated to 0x%x\n", val);
	}
	return count;
}

static DEVICE_ATTR_RO(temp);
static DEVICE_ATTR_WO(control);

static int sensor_probe(struct platform_device *pltdvc)
{
	struct sensor_dev *pdata_dev;
	struct resource *mem_res;
	void __iomem *base_addr;

	pr_info("sensor probe function called for device: %s\n", pltdvc->name);

	/* Allocate driver private data */
	pdata_dev = devm_kzalloc(&pltdvc->dev, sizeof(*pdata_dev), GFP_KERNEL);
	if (!pdata_dev)
		return -ENOMEM;

	/* Get memory resource */
	mem_res = platform_get_resource(pltdvc, IORESOURCE_MEM, 0);
	if (!mem_res)
	{
		pr_err("Failed to get memory resource\n");
		return -ENOENT;
	}

	pdata_dev->mem_res = mem_res;
	pr_info("Memory resource: %pa - %pa\n", &mem_res->start, &mem_res->end);

	/* Map physical memory */
	base_addr = devm_ioremap(&pltdvc->dev, mem_res->start, resource_size(mem_res));
	if (!base_addr)
	{
		pr_err("Failed to ioremap memory\n");
		return -ENOMEM;
	}

	pdata_dev->base_addr = base_addr;
	pdata_dev->control_value = 0;

	/* Store driver private data */
	platform_set_drvdata(pltdvc, pdata_dev);

	/* Create sysfs attributes */
	device_create_file(&pltdvc->dev, &dev_attr_temp);
	device_create_file(&pltdvc->dev, &dev_attr_control);

	return 0;
}

static int sensor_remove(struct platform_device *pltdvc)
{
	pr_info("sensor remove function called for device: %s\n", pltdvc->name);

	device_remove_file(&pltdvc->dev, &dev_attr_temp);
	device_remove_file(&pltdvc->dev, &dev_attr_control);

	return 0;
}

static const struct of_device_id sensor_of_match[] = {
	{.compatible = "temp-sensor,anis"},
	{}};
MODULE_DEVICE_TABLE(of, sensor_of_match);

static struct platform_driver sensor_pltdrv = {
	.driver = {
		.name = PLATFORM_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sensor_of_match},
	.probe = sensor_probe,
	.remove = sensor_remove,
};

module_platform_driver(sensor_pltdrv);

MODULE_LICENSE("GPL");
