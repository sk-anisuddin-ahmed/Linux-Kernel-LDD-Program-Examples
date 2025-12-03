#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include "queue_kernel.h"

void queue_init(queue *q)
{
    q->front = q->rear = q->count = 0;
    spin_lock_init(&q->lock);
}
EXPORT_SYMBOL(queue_init);

int enqueue(queue *q, char item)
{
    int ret = 0;
    spin_lock(&q->lock);

    if (q->count == QUEUE_SIZE) {
        ret = -1;
    } else {
        q->items[q->rear] = item;
        q->rear = (q->rear + 1) % QUEUE_SIZE;
        q->count++;
    }

    spin_unlock(&q->lock);
    return ret;
}
EXPORT_SYMBOL(enqueue);

int dequeue(queue *q, char *item)
{
    int ret = 0;
    spin_lock(&q->lock);

    if (q->count == 0) {
        ret = -1;
    } else {
        *item = q->items[q->front];
        q->front = (q->front + 1) % QUEUE_SIZE;
        q->count--;
    }

    spin_unlock(&q->lock);
    return ret;
}
EXPORT_SYMBOL(dequeue);

void queue_empty(queue *q)
{
    char item;
    while (dequeue(q, &item) == 0) 
	{
		
    }
	q->front = q->rear = q->count = 0;
}
EXPORT_SYMBOL(queue_empty);

static int __init queue_module_init(void)
{
    pr_info("Queue Kernel Module loaded\n");
    return 0;
}

static void __exit queue_module_exit(void)
{
    pr_info("Queue Kernel Module unloaded\n");
}

module_init(queue_module_init);
module_exit(queue_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Queue Kernel");
