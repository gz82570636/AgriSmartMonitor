#ifndef _DATA_GLOBAL_
#define _DATA_GLOBAL_

/*********************************************************
  data_global.h : 

  全局的宏定义#define
  全局的线程函数声明
  全局的设备节点声明
  全局的消息队列发送函数外部extern声明
  全局的消息队列传递的结构体信息声明

 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <syscall.h>

#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include <stdlib.h>

//消息队列长度
#define QUEUE_MSG_LEN 32
//监测数量
#define MONITOR_NUM 2

//dev的地方
#define		ZIGBEE_DEV 		 "/dev/ttyUSB0"
#define		BEEPER_DEV 		 "/dev/fsbeeper0"
#define		LED_DEV    		 "/dev/fsled0"
#define 	MPU6050_DEV  	 "/dev/fsmpu6050"
#define 	ADC_DEV    		 "/dev/fsadc0"

unsigned char tube_cmb;
unsigned char buzzer_cmd;
unsigned char led_cmd;
unsigned char fan_cmd;//风扇
unsigned char temp_cmd;
unsigned char seg_cmd;


typedef  unsigned char uint8_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;

//zigbee上传的数据
//考虑到内存对齐的问题
struct makeru_zigbee_info{
	uint8_t head[3]; //标识位: 'm' 's' 'm'  makeru-security-monitor  
	uint8_t type;	 //数据类型  'z'---zigbee  'a'---a9
	float temperature; //温度
	float humidity;  //湿度
	float tempMIN;//温度下限
	float tempMAX;//温度上限 
	float humidityMIN;   //湿度下限
	float humidityMAX;   //湿度上限
    uint32_t reserved[2]; //保留扩展位，默认填充0
	//void *data;  内核预留的扩展接口  参考
};

//a9数据结构体需要上传的数据
struct makeru_a9_info
{
    /* data */
    uint8_t head[3];////标识位: 'm' 's' 'm'  makeru-security-monitor  
    uint8_t type;//数据类型  'z'---zigbee  'a'---a9
    float adc;
	float gyrox;  
	float gyroy;
	float gyroz;
	float aacx;  
	float aacy;
	float aacz;
	float temp;
	uint32_t reserved[2]; 
};

//环境数据结构体
//房子里面的两个硬件
struct makeru_env_data{
	struct makeru_a9_info       a9_info;  
	struct makeru_zigbee_info   zigbee_info;
	uint32_t reserved[2]; //������չλ��Ĭ�����0
};

//监测数据结构体
//比如我可以设置这是1号房子这是二号房子
struct env_info_client_addr{
    //这是一号房子的监测数据
	struct makeru_env_data  monitor_no[MONITOR_NUM];//这里面存放的如果是1就是1号房子的监测数据
};

//线程函数声明,表示每个线程里面的函数
extern void *pthread_client_request (void *arg);
extern void *pthread_led(void *arg);
extern void *pthread_refresh(void *arg);
extern void *pthread_fan(void *arg);
//extern void *pthread_sqlite(void *arg);
extern void *pthread_transfer(void *arg);
extern void *pthread_buzzer(void *arg);
extern void *pthread_music(void *arg);
extern void *pthread_dh_mon(void *arg);

//发送消息队列
extern int send_msg_queue(long type,unsigned char text);

//消息队列结构体,用于数据的下发控制a9或者zigbee硬件使用
struct msg
{
	long type;//从消息队列接收消息时用于判断的消息类型
	long msgtype;//具体的消息类型
	unsigned char text[QUEUE_MSG_LEN];//消息正文
};

#endif
