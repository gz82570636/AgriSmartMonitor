#include "data_global.h"
#include "chardev.h"


extern pthread_mutex_t mutex_dh_mon;
extern pthread_cond_t cond_dh_mon;

//用来获取温湿度zb的温湿度信息
extern struct env_info_client_addr sm_all_env_info;
extern pthread_t id_music;

extern float dh_temp,dh_humidity;


extern int temMAX,temMIN,humMAX,humMIN;
extern int music_flag;

int beepfd;

void *pthread_dh_mon(void *arg)
{
    beep_desc_t beep;
   printf("pthread_dh_mon start...");  
   int i = 0;
    beepfd = open(BEEPER_DEV,O_RDWR | O_NONBLOCK);
    if (beepfd == -1)
    {
        perror("open leddev failed");
        exit(1);
    }

    while (1)
    {
        pthread_mutex_lock(&mutex_dh_mon);
        pthread_cond_wait(&cond_dh_mon,&mutex_dh_mon);

        // 温度报警
        if(temMIN > dh_temp|| dh_temp > temMAX || dh_humidity > humMAX || dh_humidity < humMIN)
        {
            printf("tem and hum is out of range\n");
            if(music_flag == 1)
            {
                void *retv;
                pthread_cancel(id_music);
                pthread_join(id_music,&retv);
                if(retv == PTHREAD_CANCELED)
                {
                    printf("pthread_music canceled\n");
                }
            }
            ioctl(beepfd,BEEP_ON,&beep);
            sleep(3);
            ioctl(beepfd,BEEP_OFF,&beep);
        }
        pthread_mutex_unlock(&mutex_dh_mon);
    }
    close(beepfd);
}