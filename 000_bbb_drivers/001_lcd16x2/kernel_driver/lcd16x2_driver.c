#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

static int rs_gpio, en_gpio;
static int d4_gpio, d5_gpio, d6_gpio, d7_gpio;

#define RS(x) gpio_set_value(rs_gpio, x)
#define EN(x) gpio_set_value(en_gpio, x)
#define D4(x) gpio_set_value(d4_gpio, x)
#define D5(x) gpio_set_value(d5_gpio, x)
#define D6(x) gpio_set_value(d6_gpio, x)
#define D7(x) gpio_set_value(d7_gpio, x)

static void lcd_enable_pulse(void)
{
    EN(1);
    mdelay(1);
    EN(0);
    mdelay(2);
}

static void lcd_write_char(uint8_t value)
{
    D4((value >> 4) & 0x01);
    D5((value >> 5) & 0x01);
    D6((value >> 6) & 0x01);
    D7((value >> 7) & 0x01);
    lcd_enable_pulse();

    D4((value >> 0) & 0x01);
    D5((value >> 1) & 0x01);
    D6((value >> 2) & 0x01);
    D7((value >> 3) & 0x01);
    lcd_enable_pulse();
}

static void lcd_command(uint8_t value)
{
    printk(KERN_DEBUG "lcd16x2: Sending command 0x%02X\n", value);
    RS(0);
    lcd_write_char(value);
}

static void lcd_write_8bit(uint8_t value)
{
    RS(1);
    lcd_write_char(value);
}

static void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t row_offsets[] = {0x00, 0x40};
    lcd_command(0x80 | (col + row_offsets[row]));
}

static void lcd_send_string(char *msg)
{
    while (*msg != '\0')
    {
        lcd_write_8bit((uint8_t)*msg++);
    }
}

static void lcd_clear(void)
{
    lcd_command(0x01);
}

static void lcd_init(void)
{
    printk(KERN_INFO "lcd16x2: Initializing LCD\n");
    msleep(15);
    lcd_command(0x02);
    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x01);
    lcd_command(0x06);
    lcd_command(0x80);
    printk(KERN_INFO "lcd16x2: LCD initialization complete\n");
}

static ssize_t lcd_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    char kbuf[33];
    size_t to_copy = len > 32 ? 32 : len;

    printk(KERN_DEBUG "lcd16x2: Write called with len=%zu\n", len);
    if (copy_from_user(kbuf, buf, to_copy))
        return -EFAULT;

    kbuf[to_copy] = '\0';

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string(kbuf);

    pr_info("lcd16x2: wrote '%s' (%zu bytes)\n", kbuf, len);
    return len;
}

static const struct file_operations lcd_fops = {
    .owner = THIS_MODULE,
    .write = lcd_write,
};

static struct miscdevice lcd_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "lcd16x2",
    .fops = &lcd_fops,
};

static int lcd16x2_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;

    printk(KERN_INFO "lcd16x2: Probe function called\n");
    rs_gpio = of_get_named_gpio(np, "rs-gpio", 0);
    en_gpio = of_get_named_gpio(np, "en-gpio", 0);
    d4_gpio = of_get_named_gpio(np, "d4-gpio", 0);
    d5_gpio = of_get_named_gpio(np, "d5-gpio", 0);
    d6_gpio = of_get_named_gpio(np, "d6-gpio", 0);
    d7_gpio = of_get_named_gpio(np, "d7-gpio", 0);
    printk(KERN_DEBUG "lcd16x2: GPIOs - RS:%d EN:%d D4:%d D5:%d D6:%d D7:%d\n", rs_gpio, en_gpio, d4_gpio, d5_gpio, d6_gpio, d7_gpio);

    gpio_request(rs_gpio, "rs");
    gpio_request(en_gpio, "en");
    gpio_request(d4_gpio, "d4");
    gpio_request(d5_gpio, "d5");
    gpio_request(d6_gpio, "d6");
    gpio_request(d7_gpio, "d7");

    gpio_direction_output(rs_gpio, 0);
    gpio_direction_output(en_gpio, 0);
    gpio_direction_output(d4_gpio, 0);
    gpio_direction_output(d5_gpio, 0);
    gpio_direction_output(d6_gpio, 0);
    gpio_direction_output(d7_gpio, 0);

    lcd_init();
    misc_register(&lcd_dev);
    dev_info(&pdev->dev, "lcd16x2 driver loaded\n");
    return 0;
}

static int lcd16x2_remove(struct platform_device *pdev)
{
    misc_deregister(&lcd_dev);

    gpio_free(rs_gpio);
    gpio_free(en_gpio);
    gpio_free(d4_gpio);
    gpio_free(d5_gpio);
    gpio_free(d6_gpio);
    gpio_free(d7_gpio);

    pr_info("lcd16x2 driver removed\n");
    return 0;
}

static const struct of_device_id lcd_of_match[] = {
    {.compatible = "generic,lcd16x2"},
    {}};
MODULE_DEVICE_TABLE(of, lcd_of_match);

static struct platform_driver lcd_driver = {
    .probe = lcd16x2_probe,
    .remove = lcd16x2_remove,
    .driver = {
        .name = "lcd16x2",
        .of_match_table = lcd_of_match,
    },
};

module_platform_driver(lcd_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anisuddin");
MODULE_DESCRIPTION("16x2 LCD Platform Driver");
