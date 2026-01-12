#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>

#define Dev_Count	5
#define Dev_Name	"my_dev"
#define Class_Name	"my_class"

static dev_t devt;
static struct cdev my_cdev;
static struct class *my_class;
static char drv_data[Dev_Count][4096]

#define print(fmt, ...) pr_info(fmt "\n", ##__VA_ARGS__)

static ssize_t rw_read(struct file *file, char __user *buf, size_t len, loff_t *pos)
{
	return 0;
}

static ssize_t rw_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    return 0;
}

static int rw_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int rw_release(struct inode *inode, struct file *file)
{
    return 0;
}

static const struct file_operations rw_fops = {
    .owner   = THIS_MODULE,
    .read    = rw_read,
    .write   = rw_write,
    .open    = rw_open,
    .release = rw_release,
};

static int __init dev_init(void)
{
    alloc_chrdev_region(&devt, 0, Dev_Count, Dev_Name);    
    rw_class = class_create(THIS_MODULE, Class_Name);		
	cdev_init(rw_cdev, &rw_fops);
	cdev_add(rw_cdev, devt, Dev_Count);
	
	for (int i = 0; i < Dev_Count; i++)
	{
		device_create(rw_class, NULL, MKDEV(MAJOR(devt), MINOR(devt) + i), &drv_data[i], "%s%d", Dev_Name, i);
	}   
    print("module loaded");
    return 0;
}

static void __exit dev_exit(void)
{
	int i; 
	for (i = 0; i < Dev_count; i++) 
	{
		device_destroy(my_class, MKDEV(MAJOR(devt), MINOR(devt) + i)); 
	}
	class_destroy(my_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(devt, Dev_count);
    print("module unloaded");
}

MODULE_LICENSE("GPL");

module_init(dev_init);
module_exit(dev_exit);