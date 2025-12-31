#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *dev = "/dev/lcd16x2";
    const char *msg = "Hello World";

    int fd = open(dev, O_WRONLY);
    if (fd < 0) 
	{
        perror("Failed to open device");
        return 1;
    }

    ssize_t written = write(fd, msg, strlen(msg));
    if (written < 0) 
	{
        perror("Failed to write to device");
    } 
	else 
	{
        printf("Wrote '%s' (%zd bytes) to %s\n", msg, written, dev);
    }

    close(fd);
    return 0;
}
