obj-m += proc2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CC=x86_64-linux-gnu-gcc CONFIG_DEBUG_INFO_BTF= modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
