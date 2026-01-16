#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>

struct my_node {
	int data;
    struct list_head list;    
};

LIST_HEAD(Head_Node);

static void add_node(int value)
{
    struct my_node *new;
    new = kmalloc(sizeof(*new), GFP_KERNEL);
    if (!new)
        return;

    new->data = value;
    INIT_LIST_HEAD(&new->list);

    list_add_tail(&new->list, &Head_Node);
    printk(KERN_INFO "Added node with value: %d\n", value);
}

static void traverse_list(void)
{
    struct my_node *temp;
    int count = 0;

    list_for_each_entry(temp, &Head_Node, list) 
	{
        printk(KERN_INFO "Node %d data = %d\n", count++, temp->data);
    }
    printk(KERN_INFO "Total Nodes = %d\n", count);
}

static void traverse_list_reverse(void)
{
    struct my_node *temp;
    int count = 0;

    list_for_each_entry_reverse(temp, &Head_Node, list) 
	{
        printk(KERN_INFO "Reverse Node %d data = %d\n", count++, temp->data);
    }
}

static void remove_node(int value)
{
    struct my_node *cursor, *tmp;
    list_for_each_entry_safe(cursor, tmp, &Head_Node, list) 
	{
        if (cursor->data == value) 
		{
            list_del(&cursor->list);
            printk(KERN_INFO "Removed node with value: %d\n", value);
            kfree(cursor);
        }
    }
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
    printk(KERN_INFO "Linked List Module Loaded\n");

    add_node(10);
    add_node(20);
    add_node(30);
	add_node(40);
	add_node(50);

    traverse_list();
    traverse_list_reverse();

    remove_node(20);
    traverse_list();

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
