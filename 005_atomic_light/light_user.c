#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
    _Bool state;
    uint8_t brightness;
    uint8_t temperature;
} lightStatus;

#define LED_ON             _IO('A', 0)
#define LED_OFF            _IO('B', 1)
#define LED_SET_BRIGHTNESS _IOW('C', 2, int)
#define LED_GET_STATE      _IOR('D', 3, int)
#define LED_GET_BRIGHTNESS _IOR('E', 4, int)

int main(void)
{
    int fd = open("/dev/LightDevice", O_RDWR);
    if (fd < 0) {
        perror("Light Device: Open Failure");
        return 1;
    }
    printf("Light Device: Open Success\n");

    lightStatus lightStatusRead = {0, 0, 0};
    lightStatus lightStatusWrite = {
        .state = 1,
        .brightness = 50,
        .temperature = 25
    };

    // Read current status from driver
    if (read(fd, &lightStatusRead, sizeof(lightStatus)) > 0) {
        printf("Light State       = %d\n", lightStatusRead.state);
        printf("Light Brightness  = %d\n", lightStatusRead.brightness);
        printf("Light Temperature = %d\n", lightStatusRead.temperature);
    }

    // Write new status to driver
    if (write(fd, &lightStatusWrite, sizeof(lightStatus)) > 0) {
        printf("Light Status Write Success\n");
    }

    // Turn LED ON
    if (ioctl(fd, LED_ON) < 0) 
		perror("ioctl Error: LED_ON");

    // Set brightness via ioctl
    int brightness = 100;
    if (ioctl(fd, LED_SET_BRIGHTNESS, &brightness) < 0)
        perror("ioctl Error: LED_SET_BRIGHTNESS");

    // Get LED state
    int state;
    if (ioctl(fd, LED_GET_STATE, &state) == 0)
        printf("LED state: %d\n", state);

    // Get LED brightness
    if (ioctl(fd, LED_GET_BRIGHTNESS, &brightness) == 0)
        printf("LED brightness: %d\n", brightness);

    // Turn LED OFF
    if (ioctl(fd, LED_OFF) < 0) 
		perror("ioctl Error: LED_OFF");

    close(fd);
    return 0;
}