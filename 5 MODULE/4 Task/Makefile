obj-m += chardevmodule.o

all:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) CC=x86_64-linux-gnu-gcc CONFIG_DEBUG_INFO_BTF= modules 2>&1 | grep -v -E "pahole|BTF|Entering|Leaving" || true

clean:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean > /dev/null 2>&1
