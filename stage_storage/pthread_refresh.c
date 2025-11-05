#include "data_global.h"
#include "sem.h"

#define N 1024


extern int semid;//信号量id
extern int shmid;//共享内存id
extern int msgid;//消息队列id

extern key_t shm_key;
extern key_t sem_key;
extern key_t key;

extern pthread_mutex_t mutex_client_request,
                    mutex_sqlite,
                    mutex_transfer,
                    mutex_refresh,
                    mutex_led,
                    mutex_buzzer,
                    mutex_sms,
                    mutex_process,
                    mutex_fan,
                    mutex_dh_mon;

extern pthread_cond_t cond_client_request,
                    cond_sqlite,
                    cond_transfer,
                    cond_refresh,
                    cond_led,
                    cond_buzzer,
                    cond_process,
                    cond_fan,
                    cond_sms,
                    cond_dh_mon,
                    cond_camera;

extern struct env_info_client_addr sm_all_env_info;

//消息队列的描述结构体
struct shm_addr
{
    char shm_status;//home选择1:使用中
    //是a9还是zigbee
    struct env_info_client_addr sm_all_env_info;//环境信息结构体
};

struct shm_addr *shm_buf;

//获取环境信息结构体
int file_env_info_struct(struct env_info_client_addr *rt_status,int home_id);

void *pthread_refresh(void *arg)
{
    int ret;
    //创建信号量
    sem_key = ftok("/tmp",'i');
    if(sem_key < 0)
    {
        perror("ftok");
        exit(-1);
    }
    //创建信号灯id
    semid = semget(sem_key,1,IPC_CREAT|IPC_EXCL|0666);
    
    if(semid == -1)
    {
        //存在信号灯id
        if(errno == EEXIST)
        {
            semid = semget(sem_key,1,0777);
        }
        else
        {
            perror("semget");
            exit(-1);
        }
    }
    else{//创建信号灯成功
        init_sem(semid,0,1);//初始化信号灯
        //就是把信号灯初始化为0
    }

    shm_key = ftok("/tmp",'i');
    if(shm_key < 0)
    {
        perror("ftok failed\n");
        exit(-1);
    }

    //创建共享内存
    shmid = shmget(shm_key,N,IPC_CREAT|IPC_EXCL|0666);
    if(shmid == -1)
    {
        if(errno == EEXIST)//存在共享内存id
        {
            shmid = shmget(shm_key,N,0777);
        }
        else
        {
            perror("shmget");
            exit(-1);
        }
    }

    //获取共享内存
    //把指定的共享内存映射到进程的地址空间用于访问
    shm_buf = (struct shm_addr *)shmat(shmid,NULL,0);
    if(shm_buf == (void *)-1)
    {
        perror("shmat");
        exit(-1);
    }
    printf("pthread_refresh start\n");

#if 1
//刷新数据
    //清零
    bzero(shm_buf,sizeof(struct shm_addr));
    while(1)
    {
        sem_p(semid,0);//P操作-1
        shm_buf->shm_status =1;
        //printf("shm_status = %d\n",shm_buf->shm_status);
        int home_id = 1;
        //file_env_info_struct(&shm_buf->sm_all_env_info,home_id);//模拟数据上传
        shm_buf->sm_all_env_info.monitor_no[home_id] = sm_all_env_info.monitor_no[home_id];
        //printf("share buf to html\n");
        sleep(1);
        sem_v(semid,0);//V操作+1
        //唤醒tranfer的wait的条件,就是当他把数据上传到共享内存以后
        pthread_cond_signal(&cond_transfer);
        //监控温室读是否在阈值内
        pthread_cond_signal(&cond_dh_mon);
    }

#endif
}


//获取环境信息结构体
int file_env_info_struct(struct env_info_client_addr *rt_status,int home_id)
{
   int env_info_size = sizeof(struct env_info_client_addr);
    printf("env_info_size = %d.\n",env_info_size);

   	rt_status->monitor_no[home_id].zigbee_info.temperature = 10.0;
	rt_status->monitor_no[home_id].zigbee_info.tempMIN = 2.0;
	rt_status->monitor_no[home_id].zigbee_info.tempMAX = 20.0;
	rt_status->monitor_no[home_id].zigbee_info.humidity  = 20.0;
	rt_status->monitor_no[home_id].zigbee_info.humidityMIN  = 10.0;
	rt_status->monitor_no[home_id].zigbee_info.humidityMAX  = 30.0;
	rt_status->monitor_no[home_id].zigbee_info.reserved[0]  = 0.01;
	rt_status->monitor_no[home_id].zigbee_info.reserved[1]  = -0.01;


	rt_status->monitor_no[home_id].a9_info.adc  = 9.0;
	rt_status->monitor_no[home_id].a9_info.gyrox  = -14.0;
	rt_status->monitor_no[home_id].a9_info.gyroy  = 20.0;
	rt_status->monitor_no[home_id].a9_info.gyroz  = 40.0;
	rt_status->monitor_no[home_id].a9_info.aacx  = 642.0;
	rt_status->monitor_no[home_id].a9_info.aacy  = -34.0;
	rt_status->monitor_no[home_id].a9_info.aacz  = 5002.0;
	rt_status->monitor_no[home_id].a9_info.temp  = 32.0;
	rt_status->monitor_no[home_id].a9_info.reserved[0]  = 0.01;
	rt_status->monitor_no[home_id].a9_info.reserved[1]  = -0.01;

    return 0;
}
