MOD_NAME = task
obj-m := $(MOD_NAME).o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	ls -alF
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	ls -alF
load:
	sudo insmod $(MOD_NAME).ko
	sudo dmesg | tail -n 10
unload:
	sudo rmmod $(MOD_NAME).ko
	sudo dmesg | tail -n 10
