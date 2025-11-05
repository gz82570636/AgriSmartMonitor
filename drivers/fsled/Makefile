
#KERNELDIR :=/lib/modules/$(shell uname -r)/build
KERNELDIR :=/home/chen/Linux_4412/kernel/linux-3.14
PWD  :=$(shell pwd)

CROSS_COMPILE=arm-linux-
#CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
CP=cp

ARM_DRVDIR=/opt/4412/rootfs/drv/
NFS_BOA_WWWDIR=/opt/4412/rootfs/boa/www/
NFS_BOA_CGIDIR=/opt/4412/rootfs/boa/cgi-bin/

obj-m +=demoarm.o 

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	arm-linux-gcc test.c -o led_test
	mv *.o *.mod.c *.order *.symvers obj

install:
	sudo cp led_test demoarm.ko  $(ARM_DRVDIR)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf cgi_led.cgi ledtest


