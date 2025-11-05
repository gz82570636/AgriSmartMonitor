#include "data_global.h"
#include "chardev.h"
#include <unistd.h> 


extern pthread_mutex_t mutex_sms;
extern pthread_cond_t cond_sms;
extern unsigned char buzzer_cmd;
extern pthread_t id_music;
extern int music_flag;
int beep_fd;


void *pthread_buzzer(void * arg)
{
    /*
    1. 生成key
    2.打开我们需要open的字符设备文件
    3.使用互斥锁和条件变量来再线程间通信
    4. 通过ioctl来控制led
    */
   beep_desc_t beep;
   printf("pthread_beeper start...");  
   int i = 0;
    beep_fd = open(BEEPER_DEV,O_RDWR | O_NONBLOCK);
    if (beep_fd == -1)
    {
        perror("open leddev failed");
        exit(1);
    }
   while (1)
   {
    pthread_mutex_lock(&mutex_sms);
    printf("wait cond_buzzer\n");
    pthread_cond_wait(&cond_sms,&mutex_sms);
    printf("pthread_beep_cmb = %#x\n",buzzer_cmd);
    if(buzzer_cmd == 0x51)
    {
        //ioctl(beep_fd,BEEP_ON,&beep);
        pthread_create(&id_music,NULL,pthread_music,(void *)beep_fd);
        
    }
    if(buzzer_cmd == 0x50)
    {
        void *retv;
        ioctl(beep_fd,BEEP_OFF,&beep);
        pthread_cancel(id_music);
        pthread_join(id_music,&retv);
        if(retv == PTHREAD_CANCELED)
        {
            printf("pthread_music canceled\n");
        }
        music_flag = 0;
    }
    pthread_mutex_unlock(&mutex_sms);
    }
    close(beep_fd); 
}

