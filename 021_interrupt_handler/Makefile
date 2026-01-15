obj-m += irq_handler_kernel.o

.PHONY: all kernel user clean help

all: clean kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc irq_handler_user.c -o irq_handler_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f irq_handler_user *.o *.out
