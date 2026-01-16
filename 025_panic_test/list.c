#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>

struct my_node
{
    char data[1024*1024*1024];
    struct list_head list;
};

LIST_HEAD(Head_Node);

static void add_node(int value)
{
    struct my_node *new;
    new = vmalloc(sizeof(*new), GFP_KERNEL);
    if (!new)
        return;

    new->data = value;
    INIT_LIST_HEAD(&new->list);

    list_add_tail(&new->list, &Head_Node);
    printk(KERN_INFO "Added node with value: %d\n", value);
}

static void cleanup_list(void)
{
    struct my_node *cursor, *tmp;
    list_for_each_entry_safe(cursor, tmp, &Head_Node, list)
    {
        list_del(&cursor->list);
        printk(KERN_INFO "Cleanup node with value: %d\n", cursor->data);
        kfree(cursor);
    }
}

static int __init list_init(void)
{
    int i;
    printk(KERN_INFO "Linked List Module Loaded\n");

    while (1)
    {
        add_node(i++);
    }

    return 0;
}

static void __exit list_exit(void)
{
    cleanup_list();
    printk(KERN_INFO "Linked List Module Unloaded\n");
}

module_init(list_init);
module_exit(list_exit);

MODULE_LICENSE("GPL");
