obj-m += hexdump.o init_exit_data.o kThread.o misc.o panic.o

kernel:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o *.out