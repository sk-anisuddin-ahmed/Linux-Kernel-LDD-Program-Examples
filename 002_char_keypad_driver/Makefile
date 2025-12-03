obj-m += queue_kernel.o keypad_helper_kernel.o key_kernel.o
 
all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean