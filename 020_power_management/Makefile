obj-m += power_mgmt_kernel.o

all: clean kernel user

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user:
	gcc power_mgmt_user.c -o power_mgmt_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f power_mgmt_user *.o *.out
