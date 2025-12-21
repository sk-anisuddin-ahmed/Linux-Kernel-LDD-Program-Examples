#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

/* Module parameter array: valid character keys for unlocking */
static char *user_keys[8];
static int key_count;
module_param_array(user_keys, charp, &key_count, 0644);
MODULE_PARM_DESC(user_keys, "List of keys for unlocking write permission");

#define STORAGE_TOTAL_SIZE   4096
#define STORAGE_SECTOR_SIZE  512
#define STORAGE_NUM_SECTORS  (STORAGE_TOTAL_SIZE / STORAGE_SECTOR_SIZE)

/* IOCTL commands */
#define IOCTL_LOCK_SECTOR    	_IOW('L', 0x1, int)
#define IOCTL_UNLOCK_SECTOR  	_IOW('U', 0x2, int)
#define IOCTL_GET_LOCK_INFO  	_IOR('I', 0x3, bool[STORAGE_NUM_SECTORS])
#define IOCTL_ERASE_SECTOR   	_IOW('E', 0x4, int)
#define IOCTL_MIRROR_SECTOR  	_IOW('M', 0x5, int)
#define IOCTL_BACKUP_TO_FILE 	_IOW('B', 0x6, char *)

/* In storage_mirror_kernel.c*/
extern void mirror_sector(int sector, const unsigned char *data);
extern int vblock_backup_to_file(const char *path);

/* Device globals */
static struct class *storage_class;
static struct device *storage_device;
static dev_t storage_dev_number;
static struct cdev storage_cdev;

/* Backing storage and metadata */
static unsigned char storage_buffer[STORAGE_TOTAL_SIZE];
static bool sector_lock_state[STORAGE_NUM_SECTORS];
static DEFINE_MUTEX(storage_mutex);

/* File operations */
static ssize_t storage_read(struct file *file, 
							char __user *user_buffer,
                            size_t length, 
							loff_t *offset)
{
    if (*offset >= STORAGE_TOTAL_SIZE)
        return 0;

    if (*offset + length > STORAGE_TOTAL_SIZE)
        length = STORAGE_TOTAL_SIZE - *offset;

    if (mutex_lock_interruptible(&storage_mutex))
        return -ERESTARTSYS;

    if (copy_to_user(user_buffer, storage_buffer + *offset, length)) 
	{
        mutex_unlock(&storage_mutex);
        return -EFAULT;
    }

    *offset += length;
    mutex_unlock(&storage_mutex);
    return length;
}

static ssize_t storage_write(struct file *file, 
							const char __user *user_buffer,
							size_t length, 
							loff_t *offset)
{
    if (*offset >= STORAGE_TOTAL_SIZE)
        return -ENOSPC;

    if (*offset + length > STORAGE_TOTAL_SIZE)
        length = STORAGE_TOTAL_SIZE - *offset;

    /* Check sector locks */
	{
		size_t start = (size_t)*offset;
		size_t end = start + length;
		int sector_start = start / STORAGE_SECTOR_SIZE;
		int sector_end = (end - 1) / STORAGE_SECTOR_SIZE;

		if (mutex_lock_interruptible(&storage_mutex))
			return -ERESTARTSYS;

		{
			int s;
			for (s = sector_start; s <= sector_end; s++) 
			{
				if (sector_lock_state[s]) 
				{
					mutex_unlock(&storage_mutex);
					return -EPERM; /* sector locked */
				}
			}
		}

		if (copy_from_user(storage_buffer + *offset, user_buffer, length)) 
		{
			mutex_unlock(&storage_mutex);
			return -EFAULT;
		}
	}

	*offset += length;
	mutex_unlock(&storage_mutex);
	return length;
}

static int storage_open(struct inode *inode, struct file *file)
{
    pr_info("storageDevice: opened\n");
    return 0;
}

static int storage_release(struct inode *inode, struct file *file)
{
    pr_info("storageDevice: released\n");
    return 0;
}

static long storage_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int sector_index;

    switch (cmd) 
	{
		case IOCTL_LOCK_SECTOR:
		{
			if (copy_from_user(&sector_index, (int __user *)arg, sizeof(int)))
				return -EFAULT;
			if (sector_index < 0 || sector_index >= STORAGE_NUM_SECTORS)
				return -EINVAL;
			mutex_lock(&storage_mutex);
			sector_lock_state[sector_index] = true;
			mutex_unlock(&storage_mutex);
			pr_info("storageDevice: sector %d locked\n", sector_index);
			return 0;
		}

		case IOCTL_UNLOCK_SECTOR:
		{
			struct {
				int sector : 3;
				char key;
			} unlock_req;

			if (copy_from_user(&unlock_req, (void __user *)arg, sizeof(unlock_req)))
				return -EFAULT;

			if (unlock_req.sector < 0 || unlock_req.sector >= STORAGE_NUM_SECTORS)
				return -EINVAL;

			/* Check key against module_param_array */
			{
				_Bool valid = false;
				int i;
				for (i = 0; i < key_count; i++) 
				{
					if (user_keys[i] && (unlock_req.key == *user_keys[i]))
					{
						valid = true;
						break;
					}
				}
				if (!valid)
					return -EPERM; /* invalid key */
			}
			if (mutex_lock_interruptible(&storage_mutex))
				return -ERESTARTSYS;

			sector_lock_state[unlock_req.sector] = false;
			mutex_unlock(&storage_mutex);

			pr_info("storageDevice: sector %d unlocked with key %d\n",
					unlock_req.sector, unlock_req.key);
			return 0;
		}

		case IOCTL_GET_LOCK_INFO:
		{
			int i;
			bool lock_info[STORAGE_NUM_SECTORS];
			mutex_lock(&storage_mutex);
			
			for (i = 0; i < STORAGE_NUM_SECTORS; i++)
				lock_info[i] = sector_lock_state[i];
			mutex_unlock(&storage_mutex);

			if (copy_to_user((bool __user *)arg, lock_info, sizeof(lock_info)))
				return -EFAULT;
			return 0;
		}

		case IOCTL_ERASE_SECTOR:
		{
			if (copy_from_user(&sector_index, (int __user *)arg, sizeof(int)))
				return -EFAULT;
			if (sector_index < 0 || sector_index >= STORAGE_NUM_SECTORS)
				return -EINVAL;
			mutex_lock(&storage_mutex);
			if (sector_lock_state[sector_index]) 
			{
				mutex_unlock(&storage_mutex);
				return -EPERM; /* cannot erase locked sector */
			}
			memset(storage_buffer + (sector_index * STORAGE_SECTOR_SIZE), 0, STORAGE_SECTOR_SIZE);
			mutex_unlock(&storage_mutex);
			pr_info("storageDevice: sector %d erased\n", sector_index);
			return 0;
		}
			
		case IOCTL_MIRROR_SECTOR:
		{
			int sector_index;
			if (copy_from_user(&sector_index, (int __user *)arg, sizeof(int)))
				return -EFAULT;
			if (sector_index < 0 || sector_index >= STORAGE_NUM_SECTORS)
				return -EINVAL;

			/* Copy from storage_buffer into mirror_buffer */
			mirror_sector(sector_index,
						  storage_buffer + (sector_index * STORAGE_SECTOR_SIZE));
			return 0;
		}

		case IOCTL_BACKUP_TO_FILE:
		{
			char path[256];
			if (copy_from_user(path, (char __user *)arg, sizeof(path)))
				return -EFAULT;

			return vblock_backup_to_file(path);
		}

		default:
			return -ENOTTY;
    }
}

static const struct file_operations storage_fops = {
    .owner          = THIS_MODULE,
    .read           = storage_read,
    .write          = storage_write,
    .open           = storage_open,
    .release        = storage_release,
    .unlocked_ioctl = storage_ioctl,
    .llseek         = default_llseek,
};

static int __init storage_driver_init(void)
{
    int ret;
	
	if (key_count == 0) 
	{
        static char *default_keys[] = {"A","B","C","D","E","F","G","H"};
		int i;
        for (i = 0; i < 8; i++) 
		{
            user_keys[i] = default_keys[i];
        }
        key_count = 8;
        pr_info("storageDevice: using default keys A–H\n");
    } 
	else 
	{
        pr_info("storageDevice: %d user keys provided\n", key_count);
    }

    memset(storage_buffer, 0, sizeof(storage_buffer));
    memset(sector_lock_state, 0, sizeof(sector_lock_state));

    ret = alloc_chrdev_region(&storage_dev_number, 0, 1, "storageDevice");
    if (ret) 
	{
        pr_err("storageDevice: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&storage_cdev, &storage_fops);
    storage_cdev.owner = THIS_MODULE;

    ret = cdev_add(&storage_cdev, storage_dev_number, 1);
    if (ret) 
	{
        pr_err("storageDevice: cdev_add failed\n");
        unregister_chrdev_region(storage_dev_number, 1);
        return ret;
    }

    storage_class = class_create(THIS_MODULE, "storage_class");
    if (IS_ERR(storage_class)) 
	{
        ret = PTR_ERR(storage_class);
        cdev_del(&storage_cdev);
        unregister_chrdev_region(storage_dev_number, 1);
        return ret;
    }

    storage_device = device_create(storage_class, NULL, storage_dev_number, NULL, "storageDevice");
    if (IS_ERR(storage_device)) 
	{
        ret = PTR_ERR(storage_device);
        class_destroy(storage_class);
        cdev_del(&storage_cdev);
        unregister_chrdev_region(storage_dev_number, 1);
        return ret;
    }

    pr_info("storageDevice: driver initialized (major=%d minor=%d)\n", MAJOR(storage_dev_number), MINOR(storage_dev_number));
    return 0;
}

static void __exit storage_driver_exit(void)
{
    device_destroy(storage_class, storage_dev_number);
    class_destroy(storage_class);
    cdev_del(&storage_cdev);
    unregister_chrdev_region(storage_dev_number, 1);
    pr_info("storageDevice: driver unloaded\n");
}

module_init(storage_driver_init);
module_exit(storage_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Sector-based 4KB storage driver with lock/unlock/erase support");