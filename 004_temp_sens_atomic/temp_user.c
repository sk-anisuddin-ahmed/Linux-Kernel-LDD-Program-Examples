#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define TEMP_SET_HIGH     _IOW('A', 1, int)
#define TEMP_SET_LOW      _IOW('B', 2, int)
#define TEMP_GET_CURRENT  _IOR('C', 3, int)
#define TEMP_GET_ALERTS   _IOR('D', 4, int)

int main(void)
{
    int fd = open("/dev/TempSensor", O_RDWR);
    if (fd < 0) 
	{
        perror("Open /dev/TempSensor failed");
        return 1;
    }
    printf("TempSensor: device opened\n");

    int high = 30, low  = 20;
    if (ioctl(fd, TEMP_SET_HIGH, &high) < 0)
        perror("ioctl TEMP_SET_HIGH");
    if (ioctl(fd, TEMP_SET_LOW, &low) < 0)
        perror("ioctl TEMP_SET_LOW");

    // Write a new temperature
    int new_temp = 32;
    if (write(fd, &new_temp, sizeof(int)) != sizeof(int))
        perror("Write Error");

    // Get current temperature
    int current;
    if (ioctl(fd, TEMP_GET_CURRENT, &current) == 0)
        printf("Current temperature = %d\n", current);
    else
        perror("ioctl Error: TEMP_GET_CURRENT");

    // Get alert count
    int alerts;
    if (ioctl(fd, TEMP_GET_ALERTS, &alerts) == 0)
        printf("Alert count = %d\n", alerts);
    else
        perror("ioctl Error: TEMP_GET_ALERTS");

    close(fd);
    return 0;
}
