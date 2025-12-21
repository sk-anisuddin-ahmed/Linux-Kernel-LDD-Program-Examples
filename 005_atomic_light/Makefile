obj-m += light_kernel.o

.PHONY: all kernel user clean

all: clean kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc light_user.c -o light_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f light_user *.o *.out