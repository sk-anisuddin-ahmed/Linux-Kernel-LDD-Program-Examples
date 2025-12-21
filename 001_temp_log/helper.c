#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include<linux/string.h>

struct class *cl;
struct device *device_node;
dev_t dev_num;
struct cdev my_dev;
int temp = 20;
module_param(temp, int, 0644);

int get_temp_val(void)
{
	return temp;
}
EXPORT_SYMBOL(get_temp_val);

static int __init temp_init(void)
{		
	pr_info("helper Module loaded succesfully\n");	
	return 0;
}

static void __exit temp_exit(void)
{
	printk(KERN_INFO "helper Module unloaded \n");
}

module_init(temp_init);
module_exit(temp_exit);

MODULE_LICENSE("GPL");
