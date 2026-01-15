obj-m += sensor_pltdrv.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CFLAGS = -Wall -Wextra -O2

all: modules app

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app: sensor_app.c
	gcc $(CFLAGS) -o sensor_app sensor_app.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f sensor_app temp-send.dtbo

dtb:
	dtc -@ -I dts -O dtb -o temp-send.dtbo temp-send.dts

load: dtb
	sudo insmod sensor_pltdrv.ko
	sudo mkdir -p /sys/kernel/config/device-tree/overlays/sensor
	echo temp-send > /sys/kernel/config/device-tree/overlays/sensor/path

unload:
	echo -1 > /sys/kernel/config/device-tree/overlays/sensor/status
	sudo rmmod sensor_pltdrv

run: app
	./sensor_app

.PHONY: all modules app clean dtb load unload run
