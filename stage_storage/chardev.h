#ifndef _CHARDEV_H
#define _CHARDEV_H

#define mytype 'a'

typedef struct led_desc
{
    /* data */
    int led_num; //2345，选择什么灯
    int led_state;//0是关闭1是打开
}led_desc_t;

#define FS_LED_ON _IOW(mytype,1,led_desc_t)//0x01
#define FS_LED_OFF _IOW(mytype,0,led_desc_t)//0x00


//加速度
struct accel_data
{
	/* data */
	short x;
	short y;
	short z;
};
//角速度
struct gyro_data
{
	/* data */
	short x;
	short y;
	short z;
};

//使用联合体
union mpu6050_data
{
	/* data */
	struct accel_data accel;
	struct gyro_data gyro;
	short temp;//温度
};
//这是在
/*
//定义带读参数的ioctl命令（copy_to_user） size为类型名
#define _IOR(type,nr,size)  _IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
*/
#define MPU6050_MAGIC 'K'
#define GET_ACCEL 	_IOR(MPU6050_MAGIC,0,union mpu6050_data)
#define GET_GYRO 	_IOR(MPU6050_MAGIC,1,union mpu6050_data)
#define GET_TEMP 	_IOR(MPU6050_MAGIC,2,union mpu6050_data)


typedef struct beep_desc{
	int beep;    //2 3 4 5
	int beep_state;  //0 or 1
	int tcnt;  //占空比
	int tcmp;  //调节占空比
}beep_desc_t;

#define beeptype 'f'

#define BEEP_ON 	_IOW(beeptype,0,beep_desc_t)
#define BEEP_OFF 	_IOW(beeptype,1,beep_desc_t)
#define BEEP_FREQ 	_IOW(beeptype,2,beep_desc_t)



#endif
