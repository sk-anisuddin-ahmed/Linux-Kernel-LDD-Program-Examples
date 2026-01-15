#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
    int fd;
    char buffer[512];
    char device_path[] = "/dev/irq_demo";

    fd = open(device_path, O_RDWR);
    if (fd < 0)
        return 1;

    if (read(fd, buffer, sizeof(buffer)) > 0)
        printf("%s", buffer);

    write(fd, "T", 1);
    sleep(1);

    lseek(fd, 0, SEEK_SET);
    if (read(fd, buffer, sizeof(buffer)) > 0)
        printf("%s", buffer);

    write(fd, "W", 1);
    sleep(1);

    lseek(fd, 0, SEEK_SET);
    if (read(fd, buffer, sizeof(buffer)) > 0)
        printf("%s", buffer);

    for (int i = 0; i < 3; i++)
    {
        write(fd, "T", 1);
        sleep(0.5);
        write(fd, "W", 1);
        sleep(0.5);
    }

    lseek(fd, 0, SEEK_SET);
    if (read(fd, buffer, sizeof(buffer)) > 0)
        printf("%s", buffer);

    write(fd, "R", 1);
    sleep(1);

    lseek(fd, 0, SEEK_SET);
    if (read(fd, buffer, sizeof(buffer)) > 0)
        printf("%s", buffer);

    close(fd);
    return 0;
}