//网页端 接受 客户端的请求

#include "data_global.h"

extern int msgid;//消息队列ID
extern key_t key;//消息队列，共享内存，信号量的key值

// 线程互斥锁
extern pthread_mutex_t mutex_client_request,
                        mutex_refresh,
                        mutex_sqlite, 
                        mutex_transfer,
                        mutex_sms,
                        mutex_buzzer,
                        mutex_led,
                        mutex_fan,
                        mutex_process,
                        mutex_tube;

// 线程条件变量
extern pthread_cond_t cond_client_request,
                       cond_refresh, 
                       cond_sqlite,
                       cond_transfer,
                       cond_process,
                       cond_tube,
                       cond_sms,
                       cond_fan,
                       cond_buzzer,
                       cond_dh_mon,
                       cond_led;

// 线程id
pthread_t id_client_request,
          id_refresh,
          id_sqlite,
          id_transfer,
          id_sms,
          id_buzzer,
          id_led;

// 消息队列
struct msg msgbuf;
extern unsigned char led_cmd;
extern unsigned char buzzer_cmd;
extern unsigned char fan_cmd;//风扇
extern unsigned char temp_cmd;
extern unsigned char seg_cmd;

int temMAX = 50,temMIN = 0,humMAX = 85,humMIN = 0;

void *pthread_client_request (void *arg)
{
    int ret;
    key = ftok("/tmp",'g');//获取键值
    if(key < 0)
    {
        perror("ftok");
        exit(1);
    }
    //打开消息队列
    msgid = msgget(key,IPC_CREAT|IPC_EXCL|0666);
	if(msgid == -1)	{
		if(errno == EEXIST){
			msgid = msgget(key,0777);
		}else{
            printf("client: msgget error");
			perror("fail to msgget");
			exit(1);
		}
	}
    printf("pthread_client_request start\n");

    while(1)
    { 
        
        bzero(&msgbuf,sizeof(msgbuf));
        //接受来自网页发过来的消息,阻塞方式接受数据
        //printf("wait for client message...\n");
        //接受从.cgi通过消息队列发送的消息到msgbuf中
        ret = msgrcv(msgid,&msgbuf,sizeof(msgbuf) - sizeof(long),1L,0);
        if(ret < 0)
        {
            perror("msgrcv");
            exit(1);
        }
        //printf("GET %ldL msg\n",msgbuf.msgtype);//输出消息类型
        //printf ("text[0] = %#x\n", msgbuf.text[0]);//输出消息内容
        switch (msgbuf.msgtype)
        {
        case 1L:
            /* code */
            //打开互斥锁
            pthread_mutex_lock(&mutex_led);
            printf("led -- start\n");
            //复制来自消息队列的正文数据
            led_cmd = msgbuf.text[0];
            printf("led_cmd = %#x\n",led_cmd);
            pthread_mutex_unlock(&mutex_led);
            //通知这个线程可以继续工作了
            pthread_cond_signal(&cond_led);
            break;
        case 2L:
            /* code */
            //蜂鸣器
            printf("buzzer -- come in\n");
            //printf("buzzer -- buzzer_cmd = %#x\n",buzzer_cmd);
            pthread_mutex_lock(&mutex_sms);
            printf("buzzer -- start\n");
            buzzer_cmd = msgbuf.text[0];
            printf("buzzer_cmd = %#x\n",buzzer_cmd);
            //释放锁
            pthread_mutex_unlock(&mutex_sms);
            //唤醒线程
            pthread_cond_signal(&cond_sms);
        break;
        case 3L://四路led
            pthread_mutex_lock(&mutex_led);
			printf("hello seg\n");
			seg_cmd= msgbuf.text[0];
            printf("seg_cmd = %#x\n",seg_cmd);
			pthread_mutex_unlock(&mutex_led);
			pthread_cond_signal(&cond_led);
        break;
        case 4L://风扇
            /* code */
            //上锁,把资源锁住,其他线程不能使用,不可以让其他人来操纵
            printf("fan -- is come\n");
            pthread_mutex_lock(&mutex_fan);
            printf("fan -- start\n");
            //复制来自消息队列的正文数据
            fan_cmd = msgbuf.text[0];
            printf("fan -- fan_cmd = %#x\n",fan_cmd);
            //释放锁
            pthread_mutex_unlock(&mutex_fan);
            //唤醒线程
            pthread_cond_signal(&cond_fan);
            break;
        case 5L://温湿度
        //说明已经设置好了环境的上限下线阈值,有pthread_dh_mon来进行监控
        /*
	        printf("set env data\n");
	        printf("temMAX: %d\n",*((int *)&msgbuf.text[1]));
	        printf("temMIN: %d\n",*((int *)&msgbuf.text[5]));
	        printf("humMIX: %d\n",*((int *)&msgbuf.text[9]));
	        printf("humMAX: %d\n",*((int *)&msgbuf.text[13]));
*/
            temMAX = *((int *)&msgbuf.text[1]);
            temMIN = *((int *)&msgbuf.text[5]);
            humMIN = *((int *)&msgbuf.text[9]);
            humMAX = *((int *)&msgbuf.text[13]);
            /*
            printf("temMAX1: %d\n", temMAX);
            printf("temMIN1: %d\n", temMIN);
            printf("humMAX1: %d\n", humMAX);
            printf("humMIN1: %d\n", humMIN);
            */
            //触发查阅线程
            pthread_cond_signal(&cond_dh_mon);

            break;
        case 6L:

        /* code */
        break;
        case 7L:
        /* code */
        break;
        default:
            break;
        }
    }
}
