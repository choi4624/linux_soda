CC = gcc
obj-m := final_ver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
			$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
			rm -f *.ko
			rm -f *.o
			rm -f *.mod.*
			rm -f .*.cmd
			m