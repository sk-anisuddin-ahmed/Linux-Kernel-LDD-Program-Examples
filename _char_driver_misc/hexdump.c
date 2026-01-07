#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init spi_dump_init(void)
{
    /* Simulated SPI RX buffer */
    u8 rx_buf[16] = {
        0x9F, 0xEF, 0x40, 0x18,
        0xAA, 0x55, 0x01, 0x02,
        0x10, 0x20, 0x30, 0x40,
        0xDE, 0xAD, 0xBE, 0xEF
    };

    pr_info("SPI RX buffer dump:\n");

    print_hex_dump(KERN_INFO,
                   "spi_rx: ",
                   DUMP_PREFIX_OFFSET, /*Address Offset*/
                   16,        /* bytes per line */
                   1,         /* group size */
                   rx_buf,	/*Buf Address*/
                   sizeof(rx_buf), /*Size*/
                   true);     /* ASCII Representation*/

    return 0;
}

static void __exit spi_dump_exit(void)
{
    pr_info("SPI dump module exit\n");
}

module_init(spi_dump_init);
module_exit(spi_dump_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SPI Hex Dump Watch");