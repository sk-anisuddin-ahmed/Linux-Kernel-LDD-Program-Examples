obj-m += temp_kernel.o

.PHONY: all kernel user clean

all: clean kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc temp_user.c -o temp_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f temp_user *.o *.out