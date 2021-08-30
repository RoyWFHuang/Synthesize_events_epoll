MODULENAME := vpoll
obj-m += $(MODULENAME).o
$(MODULENAME)-y += module.o

KERNELDIR ?= /lib/modules/`uname -r`/build
PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc -Wall -o user user.c

check: all
	sudo rmmod vpoll || echo
	sudo insmod vpoll.ko
#	sudo ./trace.sh
	sudo trace-cmd record -p function_graph -g __x64_sys_epoll_wait -O funcgraph-proc ./user
	# sudo trace-cmd record -p function -P 622 -F ./user

	sudo rmmod vpoll

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	$(RM) user
