obj-m := test_mem.o 
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
CC := gcc -ggdb2
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	$(CC)  ttool.c -o ttool
clean:
	rm -f *.c~ *.o *.ko test_mem.mod.c modules.order Module.symvers .*.cmd 
	rm -rf .tmp_versions
	rm -f test_mem.ko.unsigned
	rm -f ttool
	rm -f .test_mem.c.swp
pretty: 
	indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 *.c
	rm -f *.c~

