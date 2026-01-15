obj-m += poll.o
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

all: module poll_app select_app epoll_app

module:
	make -C $(KERNEL_DIR) M=$(PWD) modules

poll_app: poll_app.c
	gcc -Wall -O2 -o poll_app poll_app.c

select_app: select_app.c
	gcc -Wall -O2 -o select_app select_app.c

epoll_app: epoll_app.c
	gcc -Wall -O2 -o epoll_app epoll_app.c

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f poll_app select_app epoll_app

.PHONY: all clean module
