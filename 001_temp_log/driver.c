#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include<linux/string.h>
#include<linux/uaccess.h>

#define THRESHOLD_CHECK _IOWR('a', 0x11, int)

int th_high = 0x22, th_low = 0x33, th_with_limit = 0x44;
extern int get_temp_val(void);

struct class *cl;
struct device *device_node;
dev_t dev_num;
struct cdev my_dev;

int temp = 0;
int threshold_high = 0;
int threshold_low = 0;
module_param(threshold_high, int , 0644);
module_param(threshold_low, int , 0644);

static ssize_t device_read(struct file *fp, char __user *usr_buf, size_t len, loff_t *off)
{
	pr_info("read succesfully \n");
	temp = get_temp_val();
	if (copy_to_user((int *)usr_buf, &temp, sizeof(temp))
		return -EFAULT;
	return 1;
}

static ssize_t device_write(struct file *fp,const char __user *usr_buf, size_t len, loff_t *off)
{
	pr_info("write completed \n");
	return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
	return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case THRESHOLD_CHECK:
			if(temp > threshold_high)
				if (copy_to_user((int __user *) arg, &th_high, sizeof(int)))
					return -EFAULT;
			else if(temp < threshold_low)
				if(copy_to_user((int __user *)arg, &th_low, sizeof(int)))
					return -EFAULT;
			else
				if (copy_to_user((int __user *)arg, &th_with_limit, sizeof(int)))
					return -EFAULT;
			
	}
	return 0;
}

struct file_operations fop = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl
};

static int __init temp_init(void)
{
	cl = class_create(THIS_MODULE, "myclass");
	if(!alloc_chrdev_region(&dev_num, 0 , 1, "char_driver"))
	{
		printk("Device number registered\n");
		printk("Major number received:%d \nMinor number : %d\n", MAJOR(dev_num), MINOR(dev_num));
		
		cdev_init(&my_dev, &fop);
		cdev_add(&my_dev, dev_num, 1);
		device_node = device_create(cl, NULL, dev_num, NULL, "mydevice");
		
		pr_info("Module loaded succesfully\n");
		pr_info("temp : %d\n", get_temp_val() );
	}
	else
	{
		pr_info("module failed to insert");
	}
	return 0;
}

static void __exit temp_exit(void)
{
	device_destroy(cl, dev_num);
	class_destroy(cl);
	cdev_del(&my_dev);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_INFO "Module unloaded \n");
}

module_init(temp_init);
module_exit(temp_exit);

MODULE_LICENSE("GPL");
