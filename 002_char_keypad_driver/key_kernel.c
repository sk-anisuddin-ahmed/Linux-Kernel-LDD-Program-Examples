#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/uaccess.h>

extern void keypad_inject_event(int);
extern void keypad_get_event(char*);
extern void keypad_clear_queue(void);

#define CLEAR_BUF _IO('a', 0x11)

struct class *cl;
struct device *device_node;
dev_t dev_num;
struct cdev my_dev;

static ssize_t device_read(struct file *fp, char __user *usr_buf, size_t len, loff_t *off)
{
	char val;
	keypad_get_event(&val);
	if (copy_to_user(usr_buf, &val, sizeof(val)))
		return -EFAULT;
	pr_info("Read Completed: %c\n", val);
	return 1;
}

static ssize_t device_write(struct file *fp, const char __user *usr_buf, size_t len, loff_t *off)
{
	char val;	
	if (copy_from_user(&val, usr_buf, sizeof(val)))
		return -EFAULT;
	keypad_inject_event(val);
	pr_info("Write Completed: %c\n", val);
	return 1;
}

static int device_open(struct inode *inode, struct file *file)
{
	pr_info("Device Opened: %s\n", __func__);
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("Device Released: %s\n", __func__);
	return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
	case CLEAR_BUF:
		keypad_clear_queue();
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
	cl = class_create(THIS_MODULE, "keypadDevClass");
	if (!alloc_chrdev_region(&dev_num, 0, 1, "keypadDevNum"))
	{
		printk("Device number registered\n");
		printk("Major number received:%d \nMinor number : %d\n", MAJOR(dev_num), MINOR(dev_num));

		cdev_init(&my_dev, &fop);
		cdev_add(&my_dev, dev_num, 1);
		device_node = device_create(cl, NULL, dev_num, NULL, "keypadDev");
		pr_info("Module loaded succesfully\n");
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
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Keypad Driver");