#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/string.h>

static struct kobject *led_rgb_light;
static char color[10] = "red";
static int brightness = 50;
static int power = 1;

static ssize_t color_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", color);
}

static ssize_t brightness_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", brightness);
}

static ssize_t power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", power);
}

static ssize_t color_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    if (count > sizeof(color))
        return -EINVAL;
    strncpy(color, buf, count);
    color[count] = '\0';
    pr_info("LED Color = %s\n", color);
    return count;
}

static ssize_t brightness_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *buf, size_t count)
{
    int ret = kstrtoint(buf, 10, &brightness);
    if (ret < 0)
        return ret;
    pr_info("Brightness = %d\n", brightness);
    return count;
}

static ssize_t power_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    int ret = kstrtoint(buf, 10, &power);
    if (ret < 0)
        return ret;
    pr_info("Power = %s\n", power ? "ON" : "OFF");
    return count;
}

static struct kobj_attribute color_attr = __ATTR(color, 0664, color_show, color_store);
static struct kobj_attribute brightness_attr = __ATTR(brightness, 0664, brightness_show, brightness_store);
static struct kobj_attribute power_attr = __ATTR(power, 0664, power_show, power_store);

static struct attribute *attrs[] = {
    &color_attr.attr,
    &brightness_attr.attr,
    &power_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int __init led_rgb_init(void)
{
	int ret;
    led_rgb_light = kobject_create_and_add("led_rgb_light", kernel_kobj);
    if (!led_rgb_light)
        return -ENOMEM;
    ret = sysfs_create_group(led_rgb_light, &attr_group);
    pr_info("RGB LED Sysfs Driver Loaded\n");
    return ret;
}

static void __exit led_rgb_exit(void)
{
    kobject_put(led_rgb_light);
    pr_info("RGB LED Sysfs Driver Unloaded\n");
}

module_init(led_rgb_init);
module_exit(led_rgb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis");
MODULE_DESCRIPTION("SysFS RGB LED");
