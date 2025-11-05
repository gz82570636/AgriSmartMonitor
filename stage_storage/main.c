#include "data_global.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>



// 释放线程资源
void release_pthread_resource(int signo);

// 线程互斥锁
extern pthread_mutex_t mutex_client_request,
                        mutex_refresh,
                        mutex_sqlite, 
                        mutex_transfer,
                        mutex_buzzer,
                        mutex_sms,
                        mutex_led,
                        mutex_process,
                        mutex_dh_mon,
                        mutex_fan;

// 线程条件变量
extern pthread_cond_t cond_client_request,
                       cond_refresh, 
                       cond_sqlite,
                       cond_transfer,
                       cond_fan,
                       cond_sms,
                       cond_dh_mon,
                       cond_process,
                       cond_buzzer,
                       cond_led;

extern int msgid; // 消息队列id
extern int shmid;//共享内存的id
extern int semid;//信号量的id

//线程id
pthread_t id_client_request,
            id_refresh,
            id_sqlite,
            id_transfer,
            id_buzzer,
            id_process,
            id_fan,
            id_music,
            id_dh_mon,
            id_led;

//一个main函数相当于一个进程这个进程创建了很多的线程,不同的线程当中处理不同的事情
int main(int argc,const char *argv[])
{
    //初始化锁
    pthread_mutex_init(&mutex_client_request,NULL);
    pthread_mutex_init(&mutex_buzzer,NULL);
    pthread_mutex_init(&mutex_led,NULL);
    pthread_mutex_init(&mutex_refresh,NULL);
    pthread_mutex_init(&mutex_sms,NULL);
    pthread_mutex_init(&mutex_dh_mon,NULL);
    //pthread_mutex_init(&mutex_sqlite,NULL);
    pthread_mutex_init(&mutex_transfer,NULL);
    pthread_mutex_init(&mutex_fan,NULL);
    pthread_mutex_init(&mutex_process,NULL);
    printf("mutex is ok\n");

    //等待接受信号，信号处理函数,当接受到ctrl+c的信号时执行清除线程锁分离线程资源等操作
    signal(SIGINT,release_pthread_resource);

    //初始化各种条件变量
	pthread_cond_init(&cond_client_request,NULL);
	pthread_cond_init(&cond_refresh,NULL);
	//pthread_cond_init(&cond_sqlite,NULL);
	pthread_cond_init(&cond_transfer,NULL);
	pthread_cond_init(&cond_sms,NULL);
	pthread_cond_init(&cond_dh_mon,NULL);
	pthread_cond_init(&cond_buzzer,NULL);
	pthread_cond_init(&cond_led,NULL);
	pthread_cond_init(&cond_fan,NULL);
	pthread_cond_init(&cond_process,NULL);
    printf("cond is ok\n");



    //create 线程
    pthread_create(&id_client_request,NULL,pthread_client_request,NULL);
    pthread_create(&id_refresh,NULL,pthread_refresh,NULL);
    //pthread_create(&id_sqlite,NULL,pthread_sqlite,NULL);
    pthread_create(&id_transfer,NULL,pthread_transfer,NULL);
    pthread_create(&id_buzzer,NULL,pthread_buzzer,NULL);
    pthread_create(&id_led,NULL,pthread_led,NULL);
    pthread_create(&id_fan,NULL,pthread_fan,NULL);
    pthread_create(&id_dh_mon,NULL,pthread_dh_mon,NULL);
    printf("id is ok\n");

    //等待线程退出
	pthread_join(id_client_request,NULL);     printf ("pthread1 \n");
	pthread_join(id_refresh,NULL);          	printf ("pthread2\n");
	//pthread_join(id_sqlite,NULL);			printf ("pthread3\n");
	pthread_join(id_transfer,NULL);			printf ("pthread4\n");
	pthread_join(id_buzzer,NULL);			printf ("pthread6\n");
	pthread_join(id_led,NULL);				printf ("pthread7\n");
	pthread_join(id_fan,NULL);				printf ("pthread8\n");
	pthread_join(id_dh_mon,NULL);				printf ("pthread9\n");


    return 0;
}


//当触发了ctrl+c信号时，会调用此函数
void release_pthread_resource(int signo) {

    //互斥锁摧毁
    pthread_mutex_destroy(&mutex_client_request);
    pthread_mutex_destroy(&mutex_refresh);
    //pthread_mutex_destroy(&mutex_sqlite);
    pthread_mutex_destroy(&mutex_transfer);
    pthread_mutex_destroy(&mutex_sms);
    pthread_mutex_destroy(&mutex_buzzer);
    pthread_mutex_destroy(&mutex_led);
    pthread_mutex_destroy(&mutex_fan);
    pthread_mutex_destroy(&mutex_process);
    pthread_mutex_destroy(&mutex_dh_mon);

    //摧毁条件
    pthread_cond_destroy(&cond_client_request);
    pthread_cond_destroy(&cond_refresh);
    //pthread_cond_destroy(&cond_sqlite);
    pthread_cond_destroy(&cond_transfer);
    pthread_cond_destroy(&cond_sms);
    pthread_cond_destroy(&cond_fan);
    pthread_cond_destroy(&cond_process);
    pthread_cond_destroy(&cond_buzzer);
    pthread_cond_destroy(&cond_led);
    pthread_cond_destroy(&cond_dh_mon);

    //分离线程资源,防止产生僵尸线程
    pthread_detach(id_client_request);
    pthread_detach(id_refresh);
    //pthread_detach(id_sqlite);
    pthread_detach(id_transfer);
    pthread_detach(id_fan);
    pthread_detach(id_process);
    pthread_detach(id_buzzer);
    pthread_detach(id_led);
    pthread_detach(id_dh_mon);

    printf("all pthread is detached\n");

    //删除消息队列
    msgctl(msgid,IPC_RMID,NULL);
    //删除共享内存
    shmctl(shmid,IPC_RMID,NULL);
    //删除信号量
    semctl(semid,1,IPC_RMID,NULL);

    exit(0);
}



