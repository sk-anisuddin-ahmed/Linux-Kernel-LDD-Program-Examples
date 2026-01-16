obj-m += kernel_timer.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

help:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) help

.PHONY: all clean help
