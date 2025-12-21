#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define THRESHOLD_CHECK _IOWR('a', 0x11, int)
#define TH_HIGH 0x22
#define TH_LOW  0x33
#define TH_WITH_IN_LIMIT 0x44

int main()
{	
	int temp = 0;
	int fd = open("/dev/mydevice", O_RDWR);

	if(fd == -1)
	{
		perror("No device file created\n");
		return -1;
	}
	 
	read(fd, &temp, sizeof(temp) );
	printf("Temp : %d\n" , temp);

	int arg = 0;

	printf("verifying the sensor reading ----\n");
	ioctl(fd, THRESHOLD_CHECK, &arg);
	if(arg == TH_HIGH)
		printf("temperature is higher than the limit\n");
	else if(arg == TH_LOW)
		printf("temperature is lower than the limit\n");	
	else if(arg == TH_WITH_IN_LIMIT)
		printf("temperature is with in limit\n");	
	
	close(fd);
		return 0;
}
