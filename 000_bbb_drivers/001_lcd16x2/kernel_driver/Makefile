KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m := lcd16x2_driver.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
