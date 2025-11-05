
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

obj-m +=mpu6050_drv.o 

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CC) test.c -o mpu6050_test
	mv *.o *.mod.c *.order *.symvers   obj

install:
	sudo cp mpu6050_test mpu6050_drv.ko  $(ARM_DRVDIR)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf mpu6050_test


