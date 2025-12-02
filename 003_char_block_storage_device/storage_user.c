#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>

#define SECTOR_SIZE 512
#define NUM_SECTORS 8

/* IOCTL commands */
#define IOCTL_LOCK_SECTOR    	_IOW('L', 0x1, int)
#define IOCTL_UNLOCK_SECTOR  	_IOW('U', 0x2, int)
#define IOCTL_GET_LOCK_INFO  	_IOR('I', 0x3, bool[NUM_SECTORS])
#define IOCTL_ERASE_SECTOR   	_IOW('E', 0x4, int)
#define IOCTL_MIRROR_SECTOR  	_IOW('M', 0x5, int)
#define IOCTL_BACKUP_TO_FILE 	_IOW('B', 0x6, char *)

struct unlock_request {
    int sector;
    char key;
};

/* Helper: write one sector */
int write_sector(int fd, int sector, const void *buf) 
{
    off_t offset = sector * SECTOR_SIZE;
    ssize_t ret = pwrite(fd, buf, SECTOR_SIZE, offset);
    if (ret != SECTOR_SIZE) 
	{
        perror("pwrite");
        return -1;
    }
    printf("Sector %d write success\n", sector);
    return 0;
}

/* Helper: read one sector */
int read_sector(int fd, int sector, void *buf) 
{
    off_t offset = sector * SECTOR_SIZE;
    ssize_t ret = pread(fd, buf, SECTOR_SIZE, offset);
    if (ret != SECTOR_SIZE) 
	{
        perror("pread");
        return -1;
    }
    printf("Sector %d read success\n", sector);
    return 0;
}

int main(void)
{
    int fd = open("/dev/storageDevice", O_RDWR);
    if (fd < 0) 
	{
        perror("open");
        return 1;
    }
    printf("Storage Device: Open Success\n");

    /* Prepare a buffer with test data */
    char wBuf[SECTOR_SIZE];
    for (int i = 0; i < SECTOR_SIZE; i++)
        wBuf[i] = (char)(i & 0xFF);

    /* Write to sector 0 */
    if (write_sector(fd, 0, wBuf) < 0) 
	{ 
		close(fd); 
		return 1; 
	}

    /* Read back sector 0 */
    char rBuf[SECTOR_SIZE];
    if (read_sector(fd, 0, rBuf) < 0) 
	{ 
		close(fd); 
		return 1; 
	}

    /* Lock sector 2 */
    int sectorLock = 2;
    if (ioctl(fd, IOCTL_LOCK_SECTOR, &sectorLock) < 0) 
	{
        perror("lock");
        close(fd);
        return 1;
    }
    printf("Sector %d locked\n", sectorLock);

    /* Unlock sector 2 */	
	struct unlock_request req;
	req.sector = 2;
	req.key = 'B';

	if (ioctl(fd, IOCTL_UNLOCK_SECTOR, &req) < 0) 
	{
		perror("unlock");
	} 
	else 
	{
		printf("Sector %d unlocked with key %c\n", req.sector, req.key);
	}

    /* Get lock info for all sectors */
    bool lockInfo[NUM_SECTORS];
    if (ioctl(fd, IOCTL_GET_LOCK_INFO, lockInfo) < 0) 
	{
        perror("get_info");
        close(fd);
        return 1;
    }
    printf("Lock status:\n");
    for (int i = 0; i < NUM_SECTORS; i++) 
	{
        printf("Sector %d: %s\n", i, lockInfo[i] ? "LOCKED" : "UNLOCKED");
    }
	
	/* Mirror Sector 2 */
    int sectorMirror = 2;
    if (ioctl(fd, IOCTL_MIRROR_SECTOR, &sectorMirror) < 0) 
	{
        perror("mirror");
        close(fd);
        return 1;
    }
    printf("Sector %d mirrored\n", sectorMirror);

    /* Backup full storage to file */
    char backupPath[64];
    strcpy(backupPath, "/tmp/storage_backup.bin");
    if (ioctl(fd, IOCTL_BACKUP_TO_FILE, backupPath) < 0) 
	{
        perror("backup_to_file");
        close(fd);
        return 1;
    }
    printf("Storage backup written to %s\n", backupPath);

    /* Erase sector 4 */
    int sectorErase = 4;
    if (ioctl(fd, IOCTL_ERASE_SECTOR, &sectorErase) < 0) 
	{
        perror("erase");
        close(fd);
        return 1;
    }
    printf("Sector %d erased\n", sectorErase);

    close(fd);
    return 0;
}
