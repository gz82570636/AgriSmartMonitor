#ifndef  MPU6050_H
#define  MPU6050_H

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




#endif
