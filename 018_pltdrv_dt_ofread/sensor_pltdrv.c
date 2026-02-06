#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>

#define PLATFORM_DRIVER_NAME "sensor_driver"

struct sensor_dev {
    void __iomem *base_addr;
    unsigned int control_value;
    unsigned int threshold;
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

    if (kstrtouint(buf, 0, &val) == 0) {
        pdata->control_value = val;
        iowrite32(val, pdata->base_addr + 0x04);
        dev_info(dev, "Control register updated to 0x%x\n", val);
    }
    return count;
}

/* Sysfs show: threshold property */
static ssize_t threshold_show(struct device *dev,
                              struct device_attribute *attr,
                              char *buf)
{
    struct sensor_dev *pdata = dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", pdata->threshold);
}

static DEVICE_ATTR_RO(temp);
static DEVICE_ATTR_WO(control);
static DEVICE_ATTR_RO(threshold);

static int sensor_probe(struct platform_device *pltdvc)
{
    struct sensor_dev *pdata_dev;
    struct device_node *np = pltdvc->dev.of_node;
    u32 base, size, thr;

    dev_info(&pltdvc->dev, "sensor probe called for device: %s\n", pltdvc->name);

    /* Allocate driver private data */
    pdata_dev = devm_kzalloc(&pltdvc->dev, sizeof(*pdata_dev), GFP_KERNEL);
    if (!pdata_dev)
        return -ENOMEM;

    /* Read custom my-reg property */
    if (of_property_read_u32_index(np, "my-reg", 0, &base) ||
        of_property_read_u32_index(np, "my-reg", 1, &size)) {
        dev_err(&pltdvc->dev, "Failed to read my-reg property\n");
        return -EINVAL;
    }

    pdata_dev->base_addr = devm_ioremap(&pltdvc->dev, base, size);
    if (!pdata_dev->base_addr) {
        dev_err(&pltdvc->dev, "Failed to ioremap memory\n");
        return -ENOMEM;
    }

    /* Read threshold property */
    if (of_property_read_u32(np, "threshold", &thr) == 0)
        pdata_dev->threshold = thr;
    else
        pdata_dev->threshold = 0;

    pdata_dev->control_value = 0;

    /* Store driver private data */
    platform_set_drvdata(pltdvc, pdata_dev);

    /* Create sysfs attributes */
    device_create_file(&pltdvc->dev, &dev_attr_temp);
    device_create_file(&pltdvc->dev, &dev_attr_control);
    device_create_file(&pltdvc->dev, &dev_attr_threshold);

    return 0;
}

static int sensor_remove(struct platform_device *pltdvc)
{
    dev_info(&pltdvc->dev, "sensor remove called for device: %s\n", pltdvc->name);

    device_remove_file(&pltdvc->dev, &dev_attr_temp);
    device_remove_file(&pltdvc->dev, &dev_attr_control);
    device_remove_file(&pltdvc->dev, &dev_attr_threshold);

    return 0;
}

static const struct of_device_id sensor_of_match[] = {
    { .compatible = "temp-sensor,anis" },
    { }
};
MODULE_DEVICE_TABLE(of, sensor_of_match);

static struct platform_driver sensor_pltdrv = {
    .driver = {
        .name = PLATFORM_DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = sensor_of_match,
    },
    .probe = sensor_probe,
    .remove = sensor_remove,
};

module_platform_driver(sensor_pltdrv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sk");
MODULE_DESCRIPTION("Sensor driver using custom my-reg and threshold properties");
