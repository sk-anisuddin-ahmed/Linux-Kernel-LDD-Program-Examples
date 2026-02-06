#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define SYSFS_GPIO_STATE_PATH "/sys/devices/platform/sensor_driver/gpio_state"
#define SYSFS_GPIO_DIR_PATH "/sys/devices/platform/sensor_driver/gpio_direction"

/* Read GPIO state from sysfs */
int read_gpio_state(int *state)
{
    int fd;
    char buf[32];
    ssize_t len;

    fd = open(SYSFS_GPIO_STATE_PATH, O_RDONLY);
    if (fd < 0)
    {
        perror("open gpio_state");
        return -1;
    }

    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (len <= 0)
    {
        perror("read gpio_state");
        return -1;
    }

    buf[len] = '\0';
    *state = strtol(buf, NULL, 0);
    return 0;
}

/* Write GPIO state value to sysfs */
int write_gpio_state(int state)
{
    int fd;
    char buf[32];
    ssize_t len;

    fd = open(SYSFS_GPIO_STATE_PATH, O_WRONLY);
    if (fd < 0)
    {
        perror("open gpio_state");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", state);
    if (write(fd, buf, len) != len)
    {
        perror("write gpio_state");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main(void)
{
    int gpio_state = 0;

    if (read_gpio_state(&gpio_state) < 0)
        return 1;

    if (write_gpio_state(1) < 0)
        return 1;

    sleep(1);
    write_gpio_state(0);

    return 0;
}
