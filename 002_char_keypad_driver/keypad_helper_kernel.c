#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "queue_kernel.h"

static queue keyQueue;

extern void queue_init(queue *q);
extern int enqueue(queue *q, char item);
extern int dequeue(queue *q, char *item);
extern void queue_empty(queue *q);

static char *keymap = NULL;
module_param(keymap, charp, 0444);
MODULE_PARM_DESC(keymap, "String of keypad characters");

void keypad_inject_event(char key)
{
    enqueue(&keyQueue, key);
}
EXPORT_SYMBOL(keypad_inject_event);

void keypad_get_event(char *key)
{
    dequeue(&keyQueue, key);
}
EXPORT_SYMBOL(keypad_get_event);

void keypad_clear_queue(void)
{
   queue_empty(&keyQueue);
}
EXPORT_SYMBOL(keypad_clear_queue);

static int __init keypad_init(void)
{
	int i;
	
    queue_init(&keyQueue);
    printk(KERN_INFO "Helper Driver: Keypad Queue - Loaded");
	
    if (keymap)
	{
        for (i = 0; keymap[i]; i++) 
		{
            enqueue(&keyQueue, keymap[i]);
            printk(KERN_INFO "Helper Driver: Enqueued param key %c\n", keymap[i]);
        }
    } 
	else 
	{
        printk(KERN_INFO "Helper Driver: No keymap provided\n");
    }
    return 0;
}

static void __exit keypad_exit(void)
{
    printk(KERN_INFO "Helper Driver: Keypad Queue - Unloaded");
}

module_init(keypad_init);
module_exit(keypad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SK AHMED");
MODULE_DESCRIPTION("Helper Driver: keypad queue + inject function");
