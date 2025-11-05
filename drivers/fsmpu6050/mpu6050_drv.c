#include <linux/module.h>          // 内核模块基础头文件，包含模块加载/卸载等宏定义
#include <linux/kernel.h>          // 内核核心功能，包含printk等函数
#include <linux/fs.h>              // 文件系统相关函数，定义file_operations等结构体
#include <linux/cdev.h>            // 字符设备管理头文件，提供cdev相关操作
#include <linux/wait.h>            // 等待队列头文件（本代码未使用，保留作为扩展）
#include <linux/sched.h>           // 进程调度相关（本代码未使用，保留作为扩展）
#include <linux/poll.h>            // 多路复用相关（本代码未使用，保留作为扩展）
#include <linux/slab.h>            // 内核内存分配函数，如kmalloc、kfree
#include <linux/mm.h>              // 内存管理相关函数
#include <linux/io.h>              // I/O内存映射函数，如ioremap、iounmap、readl、writel
#include <asm/uaccess.h>           // 用户空间内存访问函数（本代码未使用，保留作为扩展）
#include <asm/atomic.h>            // 原子操作相关（本代码未使用，保留作为扩展）
#include <linux/i2c.h>

#include "mpu6050.h"
#define SMPLRT_DIV  0x19 //陀螺仪采样率，典型值：0x07(125Hz)
#define CONFIG   0x1A //低通滤波频率，典型值：0x06(5Hz)
#define GYRO_CONFIG  0x1B //陀螺仪自检及测量范围，典型值：0xF8(不自检，+/-2000deg/s)
#define ACCEL_CONFIG 0x1C //加速计自检、测量范围，典型值：0x19(不自检，+/-G)
#define PWR_MGMT_1  0x6B //电源管理，典型值：0x00(正常启用)，需要开始工作就需要把这个值0

//加速度
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

//温度
#define TEMP_OUT_H  0x41
#define TEMP_OUT_L  0x42

//角速度
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48



// 设备号相关定义
unsigned int major;            // 主设备号（静态指定，也可动态分配）
int mpu6050_num  = 1;        // 设备数量（只创建1个总线设备）
const char *mpu6050_name = "fsmpu6050";
dev_t devno;

// 设备结构体，封装所有硬件资源和状态
struct mpu6050_dev
{
	struct cdev mydev;          // 字符设备核心结构体，用于内核管理
	struct class *cls;
	struct device *dev;

	struct i2c_client *pclt;

};

struct mpu6050_dev *pgmydev = NULL;  // 全局设备指针，动态分配

// 打开设备函数：用户空间调用open()时触发
int mpu6050_open(struct inode *pnode,struct file *pfile)
{
	// 通过container_of宏从inode的cdev成员反向找到mpu6050_dev结构体
	// 并存储到file的private_data，方便后续操作直接获取设备信息
	pfile->private_data =(void *) (container_of(pnode->i_cdev,struct mpu6050_dev,mydev));
	printk("---->%s--->%d\n",__func__,__LINE__);

	return 0;  // 打开成功
}

// 关闭设备函数：用户空间调用close()时触发
int mpu6050_close(struct inode *pnode,struct file *pfile)
{
	return 0;  // 关闭成功（无需额外操作）
}

char mpu6050_read_byte(struct i2c_client *pclt,unsigned char reg)
{
	int ret = 0;
	char txbuf[1]={reg};
	char rxbuf[1]={0};//接受读到的数据

	struct i2c_msg msg[2] = 
	{
		//发送地址和数据
		{pclt->addr,0,1,txbuf},
		{pclt->addr,I2C_M_RD,1,rxbuf}
	};

	//i2c进行的读写共同操作
	ret = i2c_transfer(pclt->adapter,msg,ARRAY_SIZE(msg));
	if(ret < 0)
	{
		printk("i2c read transfer is failed\n");
		return ret;
	}
	return rxbuf[0];

}
int mpu6050_write_byte(struct i2c_client *pclt,unsigned char reg,unsigned char data)
{
	int ret = 0;
	char txbuf[2]={reg,data};

	struct i2c_msg msg[1] = 
	{
		{pclt->addr,0,2,txbuf},
	};
	//array_size是获取数组的长度
	ret = i2c_transfer(pclt->adapter,msg,ARRAY_SIZE(msg));
	if(ret < 0)
	{
		printk("ret = %d,in mpu6050_write_byte\n",ret);
		return ret;
	}
	return 0;
}

void init_mpu6050(struct i2c_client *pclt)
{
	mpu6050_write_byte(pclt,PWR_MGMT_1,0x00);
	mpu6050_write_byte(pclt,SMPLRT_DIV,0x07);
	mpu6050_write_byte(pclt,CONFIG,0x06);
	mpu6050_write_byte(pclt,GYRO_CONFIG,0xF8);
	mpu6050_write_byte(pclt,ACCEL_CONFIG,0x19);
}

// IOCTL控制函数：用户空间通过ioctl()发送控制命令
// 参数：pfile-文件结构体；cmd-控制命令；arg-命令参数
long mpu6050_ioctl(struct file *pfile,unsigned int cmd,unsigned long arg)
{
	// 从文件结构体中获取设备结构体指针
	struct mpu6050_dev *pmydev = (struct mpu6050_dev *)pfile->private_data;
	union mpu6050_data data;

	switch(cmd)
	{
		case GET_ACCEL:   // 读加速度命令
			data.accel.x = mpu6050_read_byte(pmydev->pclt,ACCEL_XOUT_L);
			data.accel.x = mpu6050_read_byte(pmydev->pclt,ACCEL_XOUT_H) << 8;
			//data.accel.x = data.accel.x / 16384.0;

			data.accel.y = mpu6050_read_byte(pmydev->pclt,ACCEL_YOUT_H);
			data.accel.y = mpu6050_read_byte(pmydev->pclt,ACCEL_YOUT_H) << 8;
			//data.accel.y = data.accel.y / 16384.0;

			data.accel.z = mpu6050_read_byte(pmydev->pclt,ACCEL_ZOUT_H);
			data.accel.z = mpu6050_read_byte(pmydev->pclt,ACCEL_ZOUT_H) << 8;
			//data.accel.z = data.accel.z / 16384.0;
			break;
		case GET_GYRO:  // 读角速度命令
			data.gyro.x = mpu6050_read_byte(pmydev->pclt,GYRO_XOUT_L);
			data.gyro.x = mpu6050_read_byte(pmydev->pclt,GYRO_XOUT_H) << 8;
			//data.gyro.x = data.gyro.x / 16.0;

			data.gyro.y = mpu6050_read_byte(pmydev->pclt,GYRO_YOUT_H);
			data.gyro.y = mpu6050_read_byte(pmydev->pclt,GYRO_YOUT_H) << 8;
			//data.gyro.y = data.gyro.y / 16.0;

			data.gyro.z = mpu6050_read_byte(pmydev->pclt,GYRO_ZOUT_H);
			data.gyro.z = mpu6050_read_byte(pmydev->pclt,GYRO_ZOUT_H) << 8;
			//data.gyro.z = data.gyro.z / 16.0;
			break;

		case GET_TEMP:  // 读温度命令
			data.temp = mpu6050_read_byte(pmydev->pclt,TEMP_OUT_L);
			data.temp = mpu6050_read_byte(pmydev->pclt,TEMP_OUT_H) << 8;
			//data.temp = data.temp/340.0 + 36.53;
			break;
		default:          // 未知命令
			return -EINVAL;    // 返回错误
	}
	//读到用户空间里面去
	if(0 != (copy_to_user((void *)arg,&data,sizeof(data))))
	{
		printk("copy to user is failed\n");
		return -EFAULT;
	}

	return sizeof(data);  //返回读取到的字节数
}

// 文件操作集合：关联用户空间系统调用与驱动函数
struct file_operations myops = {
	.owner = THIS_MODULE,           // 模块所有者，防止模块被意外卸载
	.open = mpu6050_open,             // 关联open()系统调用
	.release = mpu6050_close,         // 关联close()系统调用
	.unlocked_ioctl = mpu6050_ioctl,  // 关联ioctl()系统调用（无大内核锁版本）
};
//id匹配
//用了id匹配他会和client里面的info里面的id进行匹配
static struct i2c_device_id mpu6050_ids[] = 
{
	{"mpu6050",0},
	{}
};

//设备树匹配
static struct of_device_id mpu6050_dts[] = 
{
	{.compatible = "invensense,mpu6050"},
	{}
};



// 模块初始化函数：insmod时自动调用，完成设备注册
static int mpu6050_probe(struct i2c_client *pclt,const struct i2c_device_id *pid)
{

	/* 步骤1：申请设备号 */
	// 先尝试静态注册设备号
	major = register_chrdev(0,mpu6050_name,&myops);
	if(major <= 0)  // 静态注册失败，尝试动态分配非0
	{
		printk("register_chrdev failed\n");
	}
	printk("register_chrdev success,major=%d\n",major);
	devno = MKDEV(major,0);  // 组合主设备号和次设备号

	/* 步骤2：分配并初始化设备结构体 */
	pgmydev = (struct mpu6050_dev *)kmalloc(sizeof(struct mpu6050_dev),GFP_KERNEL);
	if(NULL == pgmydev)  // 内存分配失败
	{
		unregister_chrdev_region(devno,mpu6050_num);  // 释放已申请的设备号
		printk("kmalloc failed\n");  // 打印错误信息
		return -1;  // 初始化失败
	}
	memset(pgmydev,0,sizeof(struct mpu6050_dev));  // 初始化内存为0

	pgmydev->pclt = pclt;

	/* 步骤3：初始化并注册字符设备 */
	cdev_init(&pgmydev->mydev,&myops);  // 关联设备与操作函数集
	pgmydev->mydev.owner = THIS_MODULE;  // 设置设备所有者
	cdev_add(&pgmydev->mydev,devno,mpu6050_num);  // 将设备添加到内核


	//自动创建设备类
	pgmydev->cls = class_create(THIS_MODULE,mpu6050_name);
	if(!pgmydev->cls)
	{
		kfree(pgmydev);//释放内存
		unregister_chrdev_region(devno,mpu6050_num);//释放设备号
		class_destroy(pgmydev->cls);
		printk("class_create error");
		return -1;
	}
	pgmydev->dev = device_create(pgmydev->cls,NULL,devno,NULL,mpu6050_name);
	if(!pgmydev->dev)
	{
		class_destroy(pgmydev->cls);
		kfree(pgmydev);
		device_destroy(pgmydev->cls,devno);
		unregister_chrdev_region(devno,mpu6050_num);//释放设备号
		printk("device_create error");
		return -1;
	}
	printk("class device create success\n");

	init_mpu6050(pgmydev->pclt);//初始化mpu6050

	return 0;  // 初始化成功
}

// 模块退出函数：rmmod时自动调用，清理资源
static int mpu6050_remove(struct i2c_client *pclt)
{

	/* 步骤2：从内核中删除字符设备 */
	cdev_del(&pgmydev->mydev);

	/* 步骤3：释放设备号 */
	unregister_chrdev_region(devno,mpu6050_num);
	class_destroy(pgmydev->cls);
	device_destroy(pgmydev->cls,devno);

	/* 步骤4：释放设备结构体内存 */
	kfree(pgmydev);
	pgmydev = NULL;  // 指针置空，防止野指针

	return 0;
}

struct i2c_driver mpu6050_driver = 
{
	/* data */
	.driver = {
		.name = "mpu6050",
		.owner = THIS_MODULE,
		.of_match_table = mpu6050_dts,
	},
	.probe = mpu6050_probe,
	.remove = mpu6050_remove,
	.id_table = mpu6050_ids,
};



#if 0
int __init mpu6050_driver_init(void)
{
	//向内核注册i2c
	i2c_register_diver(THIS_MODULE,&mpu6050_driver);
}

void __exit mpu6050_driver_exit(void)
{
	//向内核注销i2c
	i2c_del_diver(&mpu6050_driver);
}
// 注册模块初始化函数
module_init(mpu6050_driver_init);
// 注册模块退出函数
module_exit(mpu6050_driver_exit);

#endif
//上面可以用下面这个宏定义来代替
module_i2c_driver(mpu6050_driver);


// 模块许可声明（必须）：GPL协议，否则内核拒绝加载
MODULE_LICENSE("GPL");

