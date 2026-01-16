obj-m += sigdrv.o

KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

user_prog: sigusr.c
	gcc -o sigusr sigusr.c

.PHONY: all clean user_prog
