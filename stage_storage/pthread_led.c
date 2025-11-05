#include "data_global.h"
#include "chardev.h"
#include <unistd.h> 


extern pthread_mutex_t mutex_led;
extern pthread_cond_t cond_led;
extern unsigned char led_cmd;
extern unsigned char seg_cmd;


int fswaterled_control(int led_fd, int times);
int fsled_close_all(int led_fd);
int fsled_control(int led_fd, unsigned char led_control_cmd);  //发送的数字

void *pthread_led(void * arg)
{
    /*
    1. 生成key
    2.打开我们需要open的字符设备文件
    3.使用互斥锁和条件变量来再线程间通信
    4. 通过ioctl来控制led
    */
   printf("pthread_led start...\n");  
    int led_fd = open(LED_DEV,O_RDWR);
    int j,i;
    led_desc_t led;
    if (led_fd < 0)
    {
        perror("open leddev failed");
        exit(1);
    }
    
   while (1)
   {
    /* code */
    pthread_mutex_lock(&mutex_led);
    pthread_cond_wait(&cond_led,&mutex_led);
    printf("pthread_led is running...\n");
    printf("pthread_led_cmd = %#x\n",led_cmd);
    if(led_cmd == 0x41)
    {
        fswaterled_control(led_fd,2);
        led_cmd = 0x00;
    }
    if(led_cmd == 0x40)
    {
        fsled_close_all(led_fd);
        led_cmd = 0x00;
    }
    int tmp = seg_cmd & 0xf0;
    if(seg_cmd == 0x40)
    {
        fsled_close_all(led_fd);
        seg_cmd = 0x00;
    }
	if(!(tmp ^ 0x70)){
		fsled_control(led_fd,seg_cmd);  //数码管	
	}
    pthread_mutex_unlock(&mutex_led);
   }
    close(led_fd); 
}

int fsled_close_all(int led_fd)
{
	int i = 0;
	led_desc_t led;	

	for(i = 2;i < 6;i ++){
		led.led_num = i;
		ioctl(led_fd,FS_LED_OFF,&led);
		usleep(50000);
	}
    printf("led is close\n");

	return 0;
}


int fswaterled_control(int led_fd, int times)
{
	int i = 0,j = 0;
	led_desc_t led;	

	for(j = 0;j < times;j ++){
		for(i = 2;i < 6;i ++){
			led.led_num = i;
			ioctl(led_fd,FS_LED_ON,&led);
			usleep(500000);

			led.led_num = i;
			ioctl(led_fd,FS_LED_OFF,&led);
			usleep(500000);
		}
	}

	return 0;
}


int fsled_control(int led_fd, unsigned char led_control_cmd)
{
	int i = 0;
	led_desc_t led;
	led_control_cmd &= 0x0f;
	int shift_count = 1; //第0位，第1 - 3位 

	printf("led_control_cmd = %d.\n",led_control_cmd);
	fsled_close_all(led_fd);
	sleep(3);
	while(led_control_cmd){
		if(shift_count >= 5)
			break;
		if((led_control_cmd & 0x1) == 1){ //第0位开始 = LED2
			shift_count ++;  // = 2  LED2 
			printf("if shift_count :%d.\n",shift_count);
			led.led_num = shift_count; //led2 3 4 5 灯
			ioctl(led_fd,FS_LED_ON,&led);
			usleep(50000);  //让驱动响应的时间
		}else {
			shift_count ++;
			printf("else shift_count :%d.\n",shift_count);
			led.led_num = shift_count; //led2 3 4 5 灯
			ioctl(led_fd,FS_LED_OFF,&led);
			usleep(50000);
		}
		led_control_cmd >>= 1;
	}

	return 0;
}