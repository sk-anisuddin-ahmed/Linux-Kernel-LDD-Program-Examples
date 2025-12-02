#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/string.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#define MIRROR_TOTAL_SIZE   4096
#define MIRROR_SECTOR_SIZE  512
#define MIRROR_NUM_SECTORS  (MIRROR_TOTAL_SIZE / MIRROR_SECTOR_SIZE)

static unsigned char mirror_buffer[MIRROR_TOTAL_SIZE];
static DEFINE_MUTEX(mirror_mutex);
static struct semaphore mirror_read_sem;

/* Exported: copy one sector into mirror */
void mirror_sector(int sector, const unsigned char *data)
{
    if (sector < 0 || sector >= MIRROR_NUM_SECTORS)
        return;

    if (mutex_lock_interruptible(&mirror_mutex))
        return;

    memcpy(mirror_buffer + (sector * MIRROR_SECTOR_SIZE),
           data,
           MIRROR_SECTOR_SIZE);

    mutex_unlock(&mirror_mutex);
    pr_info("storageMirror: sector %d mirrored\n", sector);
}
EXPORT_SYMBOL(mirror_sector);

/* Exported: dump full mirror buffer to a file */
int vblock_backup_to_file(const char *path)
{
    struct file *filp;
    loff_t pos = 0;
    ssize_t written = 0;
    int ret = 0;

    if (!path || !*path)
        return -EINVAL;

    filp = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (IS_ERR(filp))
        return PTR_ERR(filp);

    if (mutex_lock_interruptible(&mirror_mutex)) 
	{
        filp_close(filp, NULL);
        return -ERESTARTSYS;
    }

    while (pos < MIRROR_TOTAL_SIZE) 
	{
        ssize_t rc = kernel_write(filp, mirror_buffer + pos,
                                  MIRROR_TOTAL_SIZE - pos, &pos);
        if (rc < 0) 
		{
            ret = rc;
            break;
        }
        written += rc;
    }

    mutex_unlock(&mirror_mutex);
    filp_close(filp, NULL);

    return ret ? ret : (written == MIRROR_TOTAL_SIZE ? 0 : -EIO);
}
EXPORT_SYMBOL(vblock_backup_to_file);

static int __init mirror_init(void)
{
    memset(mirror_buffer, 0, sizeof(mirror_buffer));
    sema_init(&mirror_read_sem, 1);
    pr_info("storageMirror: initialized\n");
    return 0;
}

static void __exit mirror_exit(void)
{
    pr_info("storageMirror: exited\n");
}

module_init(mirror_init);
module_exit(mirror_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Mirror/Backup module for storageDevice");