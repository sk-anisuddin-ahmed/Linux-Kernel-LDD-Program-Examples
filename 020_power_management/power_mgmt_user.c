#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
    char state[64];
    const char *sysfs_path = "/sys/bus/platform/devices/power_mgmt_demo.0/state";

    FILE *fp = fopen(sysfs_path, "r");
    if (fp == NULL)
        return 1;

    if (fgets(state, sizeof(state), fp) != NULL)
        printf("%s", state);
    fclose(fp);

    fp = fopen(sysfs_path, "w");
    fprintf(fp, "0");
    fclose(fp);
    sleep(1);

    fp = fopen(sysfs_path, "w");
    fprintf(fp, "1");
    fclose(fp);
    sleep(1);

    for (int i = 0; i < 3; i++)
    {
        fp = fopen(sysfs_path, "w");
        fprintf(fp, "0");
        fclose(fp);
        sleep(1);

        fp = fopen(sysfs_path, "w");
        fprintf(fp, "1");
        fclose(fp);
        sleep(1);
    }

    return 0;
}
