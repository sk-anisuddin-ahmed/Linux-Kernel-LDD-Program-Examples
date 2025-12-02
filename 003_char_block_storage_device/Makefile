obj-m += storage_kernel.o storage_mirror_kernel.o

.PHONY: all kernel user clean

all: kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc storage_user.c -o storage_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f storage_user *.o