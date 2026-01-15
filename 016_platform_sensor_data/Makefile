obj-m += platform_sensor_data.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	insmod platform_sensor_data.ko

remove:
	rmmod platform_sensor_data

.PHONY: all clean install remove
