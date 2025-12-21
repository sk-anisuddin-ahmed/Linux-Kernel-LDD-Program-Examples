#ifndef _QUEUE_H
#define _QUEUE_H

#define QUEUE_SIZE 64

typedef struct
{
    char items[QUEUE_SIZE];
    int front;
    int rear;
    int count;
    spinlock_t lock;
} queue;

void queue_init(queue *q);
int enqueue(queue *q, char item);
int dequeue(queue *q, char *item);
void queue_empty(queue *q);

#endif
