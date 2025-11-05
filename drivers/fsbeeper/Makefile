export ARCH=arm
export CROSS_COMPILE=arm-linux-

#KERNELDIR :=/lib/modules/$(shell uname -r)/build
KERNELDIR :=/home/chen/Linux_4412/kernel/linux-3.14
ARM_DRVDIR=/opt/4412/rootfs/drv/
PWD  :=$(shell pwd)

obj-m +=fspwmbeeper.o 

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	arm-linux-gcc test.c -o music_test
	mv *.o *.mod.c *.order *.symvers obj

install:
	sudo cp music_test *.ko $(ARM_DRVDIR)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf music_test
	sudo rm	 /opt/4412/rootfs/drv/music_test
	sudo rm	 $(ARM_DRVDIR)fspwmbeeper.ko

