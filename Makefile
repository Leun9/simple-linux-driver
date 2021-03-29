ifneq ($(KERNELRELEASE),)
obj-m := driver.o demo.o
else
KDIR := /home/lzm/WSL2-Linux-Kernel-4.19.128-microsoft-standard
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
endif