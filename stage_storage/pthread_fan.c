#include "data_global.h"
#include "linuxuart.h"

extern pthread_mutex_t mutex_fan;
extern pthread_cond_t cond_fan;
extern unsigned char fan_cmd;

void *pthread_fan(void *arg)
{
	printf("--------%s---------\n",__FUNCTION__);
	int fan_fd = open_port(ZIGBEE_DEV);
	printf("open fan is success\n");
	if(fan_fd < 0)
	{
		printf("open_failed\n");
		return NULL;
	}
	set_com_config(fan_fd,115200,8,'N',1);
	
	char cmdbuf[4] = {0};
	while(1)
	{
		pthread_mutex_lock(&mutex_fan);
		//让他重新进入阻塞
		pthread_cond_wait(&cond_fan,&mutex_fan);
		
		if(fan_cmd == 0x21)
		{
			strcpy(cmdbuf,"11\n");
			printf("cmdbuf is %s\n",cmdbuf);
			write(fan_fd,cmdbuf,sizeof(cmdbuf));
			sleep(2);
		}
		if(fan_cmd == 0x20)
		{
			strcpy(cmdbuf,"00\n");
			printf("cmdbuf is %s\n",cmdbuf);
			write(fan_fd,cmdbuf,sizeof(cmdbuf));
			sleep(2);
		}
		pthread_mutex_unlock(&mutex_fan);
		//sleep(1);
	}

}
