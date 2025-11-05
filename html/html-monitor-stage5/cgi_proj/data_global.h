#ifndef __DATA_GLOBAL__H__
#define __DATA_GLOBAL__H__


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


/*********************************************************
  data_global.h : 

  ȫ�ֵĺ궨��#define
  ȫ�ֵ��̺߳�������
  ȫ�ֵ��豸�ڵ�����
  ȫ�ֵ���Ϣ���з��ͺ����ⲿextern����
  ȫ�ֵ���Ϣ���д��ݵĽṹ����Ϣ����

 *********************************************************/


/*********************************************************
  ȫ�ֵĺ궨��
 *********************************************************/

#define MONITOR_NUM 2
#define QUEUE_MSG_LEN  32

#define		ZIGBEE_DEV 		 "/dev/ttyUSB0"
#define		BEEPER_DEV 		 "/dev/fsbeeper0"
#define		LED_DEV    		 "/dev/fsled0"
#define		ADC_DEV    		 "/dev/fsadc0"
#define		MPU6050_DEV    	 "/dev/fsmpu6050"


/*********************************************************
  ȫ�ֵĽṹ������
 *********************************************************/

typedef  unsigned char uint8_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;

//���ǵ��ڴ���������
struct makeru_zigbee_info{
	uint8_t head[3]; //��ʶλ: 'm' 's' 'm'  makeru-security-monitor  
	uint8_t type;	 //��������  'z'---zigbee  'a'---a9
	float temperature; //�¶�
	float humidity;  //ʪ��
	float tempMIN;//�¶�����
	float tempMAX;//�¶����� 
	float humidityMIN;   //ʪ������
	float humidityMAX;   //ʪ������
	uint32_t reserved[2]; //������չλ��Ĭ�����0
};

struct makeru_a9_info{
	uint8_t head[3]; //��ʶλ: 'm' 's' 'm'  makeru-security-monitor  
	uint8_t type;	 //��������  'z'---zigbee  'a'---a9
	float adc;
	float gyrox;   //����������
	float gyroy;
	float gyroz;
	float aacx;  //���ټ�����
	float aacy;
	float aacz;
	float temp;
	uint32_t reserved[2]; //������չλ��Ĭ�����0
};

struct makeru_env_data{
	struct makeru_a9_info       a9_info;    
	struct makeru_zigbee_info   zigbee_info;
	uint32_t reserved[2]; //������չλ��Ĭ�����0
};

//���м���������Ϣ�ṹ��
struct env_info_client_addr
{
	struct makeru_env_data  monitor_no[MONITOR_NUM];	//����  �ϼ�---�¼�
};


/*********************************************************
  ȫ�ֵ��ⲿ�̺߳�������
 *********************************************************/

extern void *pthread_client_request (void *arg); //����CGI �ȵ�����
extern void *pthread_refresh(void *arg);              //ˢ�¹����ڴ������߳�
extern void *pthread_sqlite(void *arg);                 //���ݿ��̣߳��������ݿ������
extern void *pthread_transfer(void *arg);           //����ZigBee�����ݲ�����
extern void *pthread_sms(void *arg);                //���Ͷ����߳�
extern void *pthread_buzzer(void *arg);          //�����������߳�
extern void *pthread_led(void *arg);                 //led�ƿ����߳�


extern int send_msg_queue(long type,unsigned char text);

/*********************************************************
  ȫ�ֵ���Ϣ���д��ݵĽṹ������
 *********************************************************/

//��Ϣ���нṹ��
struct msg
{
	long type;//����Ϣ���н�����Ϣʱ�����жϵ���Ϣ����
	long msgtype;//�������Ϣ����
	unsigned char text[QUEUE_MSG_LEN];//��Ϣ����
};

#endif 


