#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

#define PLATFORM_DRIVER_NAME "sensor_driver"

/* Sysfs show: read GPIO state */
static ssize_t gpio_state_show(struct device *dev,
                               struct device_attribute *attr,
                               char *buf)
{
    struct gpio_desc *gpio_desc = dev_get_drvdata(dev);
    int state = gpiod_get_value(gpio_desc);
    return sprintf(buf, "%d\n", state);
}

/* Sysfs store: write GPIO value */
static ssize_t gpio_state_store(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf,
                                size_t count)
{
    struct gpio_desc *gpio_desc = dev_get_drvdata(dev);
    unsigned int val;

    if (kstrtouint(buf, 0, &val) == 0)
    {
        gpiod_set_value(gpio_desc, val ? 1 : 0);
    }
    return count;
}

/* Sysfs show: GPIO direction */
static ssize_t gpio_direction_show(struct device *dev,
                                   struct device_attribute *attr,
                                   char *buf)
{
    struct gpio_desc *gpio_desc = dev_get_drvdata(dev);
    int direction = gpiod_get_direction(gpio_desc);
    return sprintf(buf, "%s\n", direction ? "in" : "out");
}

static DEVICE_ATTR_RW(gpio_state);
static DEVICE_ATTR_RO(gpio_direction);

static int sensor_probe(struct platform_device *pltdvc)
{
    struct gpio_desc *gpio_desc;

    /* Get GPIO descriptor from device tree */
    gpio_desc = devm_gpiod_get(&pltdvc->dev, "sensor", GPIOD_OUT_LOW);
    if (IS_ERR(gpio_desc))
    {
        dev_err(&pltdvc->dev, "Failed to get GPIO\n");
        return PTR_ERR(gpio_desc);
    }

    /* Store GPIO descriptor directly */
    dev_set_drvdata(&pltdvc->dev, gpio_desc);

    /* Create sysfs attributes */
    device_create_file(&pltdvc->dev, &dev_attr_gpio_state);
    device_create_file(&pltdvc->dev, &dev_attr_gpio_direction);

    return 0;
}

static int sensor_remove(struct platform_device *pltdvc)
{
    struct sensor_dev *pdata = platform_get_drvdata(pltdvc);
    device_remove_file(&pltdvc->dev, &dev_attr_gpio_direction);

    return 0;
}

static const struct of_device_id sensor_of_match[] = {
    {.compatible = "temp-sensor,anis"},
    device_remove_file(&pltdvc->dev, &dev_attr_gpio_state
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
