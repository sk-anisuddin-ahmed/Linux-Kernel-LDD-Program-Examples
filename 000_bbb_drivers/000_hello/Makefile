obj-m += hello.o

.PHONY: all kernel user clean

all: clean kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc hello.c -o hello

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f hello *.o *.out