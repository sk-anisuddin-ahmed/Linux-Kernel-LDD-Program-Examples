#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define SYSFS_TEMP_PATH "/sys/devices/platform/sensor_driver/temp"
#define SYSFS_CONTROL_PATH "/sys/devices/platform/sensor_driver/control"

/* Read sensor temperature from sysfs */
int read_temperature(unsigned int *temp)
{
    int fd;
    char buf[32];
    ssize_t len;

    fd = open(SYSFS_TEMP_PATH, O_RDONLY);
    if (fd < 0)
    {
        perror("open temp");
        return -1;
    }

    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (len <= 0)
    {
        perror("read temp");
        return -1;
    }

    buf[len] = '\0';
    *temp = strtoul(buf, NULL, 0); // parse decimal or hex
    return 0;
}

/* Write sensor control value to sysfs */
int write_control(unsigned int control)
{
    int fd;
    char buf[32];
    ssize_t len;

    fd = open(SYSFS_CONTROL_PATH, O_WRONLY);
    if (fd < 0)
    {
        perror("open control");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%u", control);
    if (write(fd, buf, len) != len)
    {
        perror("write control");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main(void)
{
    unsigned int temp_value;
    unsigned int control_value = 0xDEAD;

    printf("Sensor Application Started\n");

    /* Read temperature */
    if (read_temperature(&temp_value) == 0)
        printf("Temperature register value: %u\n", temp_value);
    else
        fprintf(stderr, "Failed to read temperature\n");

    /* Write control value */
    printf("Writing control value: 0x%x (%u)\n", control_value, control_value);
    if (write_control(control_value) < 0)
        fprintf(stderr, "Failed to write control\n");

    /* Read temperature again */
    sleep(1);
    if (read_temperature(&temp_value) == 0)
        printf("Temperature register value: %u\n", temp_value);

    printf("Sensor Application Completed\n");
    return 0;
}
