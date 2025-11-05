#include <stdio.h>
#include "data_global.h"

/*********************************************************
   data_global.c :
       全局的互斥体声明
       全局的条件锁声明声明
        全局的id和key信息声明
        全局的消息队列发送函数声明
 *********************************************************/

 // 互斥体声明
pthread_mutex_t mutex_client_request,
                mutex_refresh,
                mutex_led,
                mutex_buzzer,
                mutex_tube,
                mutex_process,
                mutex_fan,
                mutex_transfer,
                mutex_dh_mon,
                mutex_sqlite,
                mutex_sms;

// 消息队列声明
pthread_cond_t cond_client_request,
              cond_refresh,
              cond_tube,
              cond_led,
              cond_buzzer,
              cond_dh_mon,
              cond_transfer,
              cond_process,
              cond_fan,
              cond_sqlite,
              cond_sms;
        
int msgid;//消息队列ID
int shmid;//共享内存ID
int semid;//信号量id

key_t key;//键值
key_t shm_key;
key_t sem_key;

// 消息结构体
struct env_info_client_addr sm_all_env_info;

//发送消息队列函数
int send_msg_queue(long type,unsigned char text)
{
    struct msg msgbuf;
    msgbuf.type = 1L;//消息类型
    msgbuf.msgtype = type;//消息内容
    msgbuf.text[0] = text;//消息内容

    //发送消息
    if(msgsnd(msgid,&msgbuf,sizeof(msgbuf) - sizeof(long),0) == -1)
    {
        perror("msgsend");
        exit(1);
    }
    return 0;
}
