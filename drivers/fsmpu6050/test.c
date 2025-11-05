/*************************************************************************
	#	 FileName	: test.c
	#	 Author		: fengjunhui 
	#	 Email		: 18883765905@163.com 
	#	 Created	: 2017年07月03日 星期一 15时48分02秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "mpu6050.h"


void delay_ms(unsigned char times)
{
	int i = 0;
	for( ;times > 0;times --)
	{
		for(i = 1;i > 0;i -- ){
			usleep(1000);
		}
	}
}


int main(int argc, const char *argv[])
{

	int i;
	int fd;
	union mpu6050_data data;

	char usrbuf[4]={0};
	char kbuf[4]={0};
	fd = open("/dev/fsmpu6050",O_RDWR);
	if(fd < 0){
		printf("open failed !\n");
		return -1;
	}
	printf("open device success! fd: %d\n",fd);
	
	read(fd,usrbuf,4);
	printf("usrbuf[2] : %c\n",usrbuf[2]);

	usrbuf[0] = '9';
	write(fd,usrbuf,4);

	 // 定义专用变量接收换算后的数据
    float accel_x, accel_y, accel_z; // 加速度（单位：g）
    float gyro_x, gyro_y, gyro_z;    // 角速度（单位：°/s）
    float temp_c;                    // 温度（单位：°C）
		

	while(1){

		printf("***********************************gyro*******************\n");
		ioctl(fd,GET_GYRO,&data);
		gyro_x = (float)data.gyro.x / 16.4;
		gyro_y = (float)data.gyro.y / 16.4;
		gyro_z = (float)data.gyro.z / 16.4;
		printf("gyro data: x = %05d, y = %05d, z = %05d\n", data.gyro.x,data.gyro.y,data.gyro.z);
		printf("gyro : x=%.1f °/s, y=%.1f °/s, z=%.1f °/s\n", gyro_x, gyro_y, gyro_z);
		sleep(1);	
		
		printf("----------------------------------accel-------------------\n");
		ioctl(fd,GET_ACCEL,&data);
		accel_x = (float)data.accel.x / 16384.0;
		accel_y = (float)data.accel.y / 16384.0;
		accel_z = (float)data.accel.z / 16384.0;
		printf("accel data: x = %05d, y = %05d, z = %05d\n", data.accel.x,data.accel.y,data.accel.z);
		printf("accel: x=%.3f g, y=%.3f g, z=%.3f g\n", accel_x, accel_y, accel_z);
		sleep(1);
		
		printf("===================================temp===================\n");
		ioctl(fd,GET_TEMP,&data);
		temp_c = (float)data.temp/340 + 36.53;
		printf("temp data:  = %05d\n",data.temp);
		printf("temp_c data: %.1f °C\n", temp_c);

		sleep(1);
	}

	close(fd);

	return 0;
}
