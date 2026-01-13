#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#define DEV0 "/dev/dual_poll0"
#define DEV1 "/dev/dual_poll1"

int main(void)
{
    int fd0, fd1, maxfd;
    fd_set readfds;
    char buf[128];
    int ret;

    // Open both devices in non-blocking mode
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

    // Highest fd + 1 for select()
    maxfd = (fd0 > fd1 ? fd0 : fd1) + 1;

    printf("Monitoring %s and %s for events...\n", DEV0, DEV1);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd0, &readfds);
        FD_SET(fd1, &readfds);

        // Block until one of the devices has data
        ret = select(maxfd, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(fd0, &readfds)) {
            memset(buf, 0, sizeof(buf));
            ret = read(fd0, buf, sizeof(buf) - 1);
            if (ret > 0) {
                buf[ret] = '\0';
                printf("[dev0] %s\n", buf);
            }
        }

        if (FD_ISSET(fd1, &readfds)) {
            memset(buf, 0, sizeof(buf));
            ret = read(fd1, buf, sizeof(buf) - 1);
            if (ret > 0) {
                buf[ret] = '\0';
                printf("[dev1] %s\n", buf);
            }
        }
    }

    close(fd0);
    close(fd1);
    return 0;
}
