#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/ioctl.h>

#define CLEAR_BUF _IO('a', 0x11)

int main()
{
	int fd = open("/dev/keypadDev", O_RDWR);

	if(fd < 0)
	{
		perror("No device file created\n");
		return -1;
	}
	
    char ch = 'a';
    if (write(fd, &ch, 1) < 0) 
	{
        perror("Failed to write to device");
        close(fd);
        return 1;
    }
	
	int myKey;
	read(fd, &myKey, sizeof(myKey));
	printf("Keypad Value : %d\n", myKey);
	 
	if ((ioctl(fd, CLEAR_BUF)) < 0)
	{
		perror("IOCTL Call Failed\n");
		return -1;
	}
	
	printf("Queue Buffer is Cleared\n");
	 
	close(fd);
	return 0;
}
