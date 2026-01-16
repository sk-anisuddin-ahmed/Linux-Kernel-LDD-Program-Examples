#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

#define REG_CURRENT_TASK _IOW('a','a',int32_t*)
#define SIGETX 44

void sig_event_handler(int signo, siginfo_t *info, void *context)
{
    if (signo == SIGETX)
        printf("Received signal from kernel: value=%d\n", info->si_int);
}

int main()
{
    int fd;
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sig_event_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGETX, &act, NULL);

    fd = open("/dev/etx_device", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    ioctl(fd, REG_CURRENT_TASK, NULL);
    printf("Registered with driver, waiting for signal...\n");

    while (1) pause();  // wait for signals

    close(fd);
    return 0;
}
