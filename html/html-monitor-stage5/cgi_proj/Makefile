CC=arm-none-linux-gnueabi-gcc
AFLAGS=-Wall -g
LDFLAGS= -L ./lib
OBJS= capture.cgi a9_beep.cgi zigbee_fan.cgi\
	  a9_led.cgi a9_seg.cgi \
	  env1.cgi setEnv.cgi set_sms.cgi

run:$(OBJS)

$(OBJS):%.cgi:%.c
	$(CC) $(AFLAGS) $< cgic.c -o $@ $(LDFLAGS)
	
install:
	$(CC) $(AFLAGS) login.c -o login.cgi
	mv *.cgi ./out/

	sudo cp  ./out/*.cgi  /opt/4412/rootfs/boa/cgi-bin
	sudo cp  *.html  /opt/4412/rootfs/boa/www

.PHONY:clean
clean:
	rm ./out/*.cgi

