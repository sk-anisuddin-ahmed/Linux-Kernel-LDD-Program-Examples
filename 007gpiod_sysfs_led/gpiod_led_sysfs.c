#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct gpio_desc *led_gpio;
static struct kobject *kobj;
static int led_state;

static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", led_state);
}

static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%d", &led_state);
    gpiod_set_value(led_gpio, led_state);
    pr_info("LED State = %d\n", led_state);
    return count;
}

static struct kobj_attribute value_attr = __ATTR(value, 0664, value_show, value_store);
static struct attribute *attrs[] = {&value_attr.attr, NULL};
static struct attribute_group attr_group = {.attrs = attrs};

static int gpio_led_probe(struct platform_device *pdev)
{
    int ret;
    pr_info("GPIO LED Platform driver probed\n");
    led_gpio = gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpio))
        return PTR_ERR(led_gpio);
    kobj = kobject_create_and_add("gpio_led", kernel_kobj);
    if (!kobj)
        return -ENOMEM;
    ret = sysfs_create_group(kobj, &attr_group);
    if (ret)
    {
        kobject_put(kobj);
        return ret;
    }
    return 0;
}

static int gpio_led_remove(struct platform_device *pdev)
{
    pr_info("GPIO LED Platform driver removed\n");
    gpiod_put(led_gpio);
    kobject_put(kobj);
    return 0;
}

static const struct of_device_id gpio_led_dt_match[] = {
    {.compatible = "anis,gpio-led"},
    {}};
MODULE_DEVICE_TABLE(of, gpio_led_dt_match);

static struct platform_driver gpio_led_driver = {
    .probe = gpio_led_probe,
    .remove = gpio_led_remove,
    .driver = {
        .name = "gpio_led_driver",
        .of_match_table = gpio_led_dt_match,
    },
};

module_platform_driver(gpio_led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");
MODULE_DESCRIPTION("GPIO Platform Driver with SysFS");
