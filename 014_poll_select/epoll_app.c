#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>

#define DEV0 "/dev/dual_poll0"
#define DEV1 "/dev/dual_poll1"

int main(void)
{
    int fd0, fd1;
    int epfd;
    struct epoll_event ev, events[2];
    char buf[128];
    int ret, nfds;

    // Open both devices in non-blocking mode
    fd0 = open(DEV0, O_RDONLY | O_NONBLOCK);
    if (fd0 < 0)
    {
        perror("open dev0");
        return 1;
    }

    fd1 = open(DEV1, O_RDONLY | O_NONBLOCK);
    if (fd1 < 0)
    {
        perror("open dev1");
        close(fd0);
        return 1;
    }

    // Create an epoll instance
    epfd = epoll_create1(0);
    if (epfd < 0)
    {
        perror("epoll_create1");
        close(fd0);
        close(fd1);
        return 1;
    }

    // Add fd0 to epoll
    ev.events = EPOLLIN;
    ev.data.fd = fd0;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd0, &ev) < 0)
    {
        perror("epoll_ctl add fd0");
        close(epfd);
        close(fd0);
        close(fd1);
        return 1;
    }

    // Add fd1 to epoll
    ev.events = EPOLLIN;
    ev.data.fd = fd1;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd1, &ev) < 0)
    {
        perror("epoll_ctl add fd1");
        close(epfd);
        close(fd0);
        close(fd1);
        return 1;
    }

    printf("Monitoring %s and %s for events using epoll...\n", DEV0, DEV1);

    while (1)
    {
        // Wait for events on all registered file descriptors
        nfds = epoll_wait(epfd, events, 2, -1); // -1 means wait indefinitely
        if (nfds < 0)
        {
            perror("epoll_wait");
            break;
        }

        // Process all events
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].events & EPOLLIN)
            {
                memset(buf, 0, sizeof(buf));
                ret = read(events[i].data.fd, buf, sizeof(buf) - 1);
                if (ret > 0)
                {
                    buf[ret] = '\0';
                    if (events[i].data.fd == fd0)
                        printf("[dev0] %s\n", buf);
                    else if (events[i].data.fd == fd1)
                        printf("[dev1] %s\n", buf);
                }
            }
        }
    }

    close(epfd);
    close(fd0);
    close(fd1);
    return 0;
}
