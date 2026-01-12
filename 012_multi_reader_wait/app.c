#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define DEVICE_FILE "/dev/my_dev"
#define BUFFER_SIZE 256

typedef struct
{
    int reader_id;
    int fd;
} reader_args_t;

void *reader_thread(void *arg)
{
    reader_args_t *args = (reader_args_t *)arg;
    char buffer[BUFFER_SIZE] = {0};
    ssize_t ret;

    printf("[Reader %d] Waiting for data...\n", args->reader_id);

    ret = read(args->fd, buffer, BUFFER_SIZE - 1);
    if (ret > 0)
    {
        buffer[ret] = '\0';
        printf("[Reader %d] Read: %s\n", args->reader_id, buffer);
    }
    else if (ret < 0)
    {
        perror("read");
    }

    close(args->fd);
    free(args);
    return NULL;
}

void writer_func(const char *message)
{
    int fd = open(DEVICE_FILE, O_WRONLY);
    if (fd < 0)
    {
        perror("open for write");
        return;
    }

    ssize_t ret = write(fd, message, strlen(message));
    if (ret > 0)
    {
        printf("[Writer] Wrote %ld bytes: %s\n", ret, message);
    }
    else
    {
        perror("write");
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    pthread_t reader1, reader2, writer;
    reader_args_t *args1, *args2;
    const char *test_message = "Hello World";

    args1 = malloc(sizeof(reader_args_t));
    args1->reader_id = 1;
    args1->fd = open(DEVICE_FILE, O_RDONLY);
    if (args1->fd < 0)
    {
        perror("open reader 1");
        return 1;
    }

    args2 = malloc(sizeof(reader_args_t));
    args2->reader_id = 2;
    args2->fd = open(DEVICE_FILE, O_RDONLY);
    if (args2->fd < 0)
    {
        perror("open reader 2");
        return 1;
    }

    pthread_create(&reader1, NULL, reader_thread, args1);
    pthread_create(&reader2, NULL, reader_thread, args2);

    sleep(1);

    writer_func(test_message);

    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    return 0;
}
