#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#define DEV0 "/dev/dual_poll0"
#define DEV1 "/dev/dual_poll1"

int main(void)
{
    int fd0, fd1;
    struct pollfd fds[2];
    char buf[128];
    int ret;

    fd0 = open(DEV0, O_RDONLY | O_NONBLOCK);
    if (fd0 < 0) {
        perror("open dev0");
        return 1;
    }

    fd1 = open(DEV1, O_RDONLY | O_NONBLOCK);
    if (fd1 < 0) {
        perror("open dev1");
        close(fd0);
        return 1;
    }

    fds[0].fd = fd0;
    fds[0].events = POLLIN;
    fds[1].fd = fd1;
    fds[1].events = POLLIN;

    printf("Waiting for events on %s and %s...\n", DEV0, DEV1);

    while (1) {
        ret = poll(fds, 2, -1);  // wait indefinitely
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            memset(buf, 0, sizeof(buf));
            ret = read(fd0, buf, sizeof(buf));
            if (ret > 0)
                printf("Event from dev0: %s\n", buf);
        }

        if (fds[1].revents & POLLIN) {
            memset(buf, 0, sizeof(buf));
            ret = read(fd1, buf, sizeof(buf));
            if (ret > 0)
                printf("Event from dev1: %s\n", buf);
        }
    }

    close(fd0);
    close(fd1);
    return 0;
}
