#ifndef _MONITOR_SEM_H_
#define _MONITOR_SEM_H_ 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// 信号量初始化
union semun{
    int val;//用于setval命令设置信号量初始值
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
								(Linux-specific) */
};

/*
 * 初始化信号量
 * @param semid 信号量集标识符
 * @param num 信号量在信号量集中的索引
 * @param val 要设置的信号量初始值
 * @return 成功返回0，失败返回-1并退出程序
 */
int init_sem(int semid,int num,int val)
{
    int ret;
    union semun mysun;
    mysun.val = val;
    //设置信号量的初始值
    ret = semctl(semid, num, SETVAL, mysun);
    if(ret < 0)
    {
        perror("semctl");
        exit(-1);
    }
    
}

// 信号量P操作,只有num传进来的是1的时候才进行P操作
int sem_p(int semid,int num)
{
    int ret;
    struct sembuf mybuf;
    mybuf.sem_num = num;
    mybuf.sem_op = -1;//P操作表示把信号量的值减1
    mybuf.sem_flg = SEM_UNDO;//释放信号量时，会调用信号量撤销函数
    //执行信号量P操作
    ret = semop(semid,&mybuf,1);
    if(ret < 0)
    {
        perror("semop");
        exit(-1);
    }
    return 0;
}
//信号V操作，只有num传进来是0的时候，才会执行V操作
int sem_v(int semid,int num)
{
    int ret;
    struct sembuf mybuf;
    mybuf.sem_num = num;
    mybuf.sem_op = 1;//V操作表示信号量的值加1
    mybuf.sem_flg = SEM_UNDO;
    ret = semop(semid,&mybuf,1);
    if(ret < 0)
    {
        perror("semop");
        exit(-1);
    }
    return 0;
}





#endif