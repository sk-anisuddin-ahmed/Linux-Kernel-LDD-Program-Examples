#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

MODULE_LICENSE("GPL");

static struct device *power_device;
static int device_state = 1;

static int power_suspend(struct device *dev)
{
    device_state = 0;
    return 0;
}

static int power_resume(struct device *dev)
{
    device_state = 1;
    return 0;
}

static int power_runtime_suspend(struct device *dev)
{
    return 0;
}

static int power_runtime_resume(struct device *dev)
{
    return 0;
}

static int power_freeze(struct device *dev)
{
    return 0;
}

static int power_thaw(struct device *dev)
{
    return 0;
}

static ssize_t state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "Device State: %s\n", device_state ? "Active" : "Suspended");
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr,
                           const char *buf, size_t count)
{
    if (buf[0] == '0')
        pm_runtime_put_sync(dev);
    else if (buf[0] == '1')
        pm_runtime_get_sync(dev);
    return count;
}

static DEVICE_ATTR_RW(state);

static struct attribute *power_attrs[] = {
    &dev_attr_state.attr,
    NULL};

static struct attribute_group power_attr_group = {
    .attrs = power_attrs,
};

static const struct dev_pm_ops power_pm_ops = {
    .suspend = power_suspend,
    .resume = power_resume,
    .freeze = power_freeze,
    .thaw = power_thaw,
    .runtime_suspend = power_runtime_suspend,
    .runtime_resume = power_runtime_resume,
};

static int power_probe(struct platform_device *pdev)
{
    int ret;

    power_device = &pdev->dev;

    ret = sysfs_create_group(&power_device->kobj, &power_attr_group);
    if (ret)
        return ret;

    pm_runtime_enable(power_device);
    return 0;
}

static int power_remove(struct platform_device *pdev)
{
    pm_runtime_disable(power_device);
    sysfs_remove_group(&power_device->kobj, &power_attr_group);
    return 0;
}

static struct platform_driver power_driver = {
    .probe = power_probe,
    .remove = power_remove,
    .driver = {
        .name = "power_mgmt_demo",
        .pm = &power_pm_ops,
    },
};

static int __init power_init(void)
{
    return platform_driver_register(&power_driver);
}

static void __exit power_exit(void)
{
    platform_driver_unregister(&power_driver);
}

module_init(power_init);
module_exit(power_exit);