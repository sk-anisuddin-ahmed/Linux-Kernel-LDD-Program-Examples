obj-m += sensor_pltdvc.o sensor_pltdrv.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: modules app

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app: sensor_app.c
	gcc -o sensor_app sensor_app.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f sensor_app

load:
	sudo insmod sensor_pltdrv.ko
	sudo insmod sensor_pltdvc.ko

unload:
	sudo rmmod sensor_pltdvc
	sudo rmmod sensor_pltdrv

.PHONY: all modules app clean load unload run
