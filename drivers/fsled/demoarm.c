#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "chardev.h"



// 定义GPIO寄存器物理地址（根据硬件手册填写，不同芯片地址不同）
#define GPX1CON 0x11000C20  // GPX1控制器配置寄存器
#define GPX1DAT 0x11000C24  // GPX1数据寄存器
#define GPX2CON 0x11000C40  // GPX2控制器配置寄存器
#define GPX2DAT 0x11000C44  // GPX2数据寄存器
#define GPF3CON 0x114001E0  // GPF3控制器配置寄存器
#define GPF3DAT 0x114001E4  // GPF3数据寄存器
//5灯直接复用gpf3控制器

unsigned int  major;//设备主设备号
unsigned int  dev_num = 1;//设备数量
dev_t devno;//设备号
const char *name = "fsled0";

struct myled_dev{ 

	struct cdev mycdev;

	struct class *cls;
	struct device *dev;
	led_desc_t led_f;

	volatile unsigned int *myled2_con;//led2控制寄存器地址
	volatile unsigned int *myled2_data;//led2数据寄存

	volatile unsigned int *myled3_con;
	volatile unsigned int *myled3_data;

	volatile unsigned int *myled4_con;
	volatile unsigned int *myled4_data;

	volatile unsigned int *myled5_con;
	volatile unsigned int *myled5_data;
};
//动态分配内内存
struct myled_dev *pmydev = NULL;


char kbuf[] = {'1','2','3','4'};

//移动文件指针
loff_t myled_llseek(struct file *filp, loff_t offset, int cnt)
{

	printk("---->%s--->%d\n",__func__,__LINE__);
	return 0;
}

//读取数据
ssize_t myled_read(struct file *filp, char __user *usrbuf, size_t count, loff_t *offset)
{
	int bytes = 0;
	printk("---->%s--->%d\n",__func__,__LINE__);

	//模拟数据	
	bytes =	copy_to_user(usrbuf,kbuf,4);
	if(bytes > 0){
		printk("copy_to_user failed!\n");
	}

	return 0;
}
//因为物理地址无法直接访问，需要映射到虚拟地址空间
void gpio_ioremap(struct myled_dev *pgmydev){
	//映射GPX2CON和GPX2DAT寄存器,到虚拟地址空间(32)
	pmydev->myled2_con = ioremap(GPX2CON,4);
	pmydev->myled2_data = ioremap(GPX2DAT,4);

	pmydev->myled3_con = ioremap(GPX1CON,4);
	pmydev->myled3_data = ioremap(GPX1DAT,4);

	pmydev->myled4_con = ioremap(GPF3CON,4);
	pmydev->myled4_data = ioremap(GPF3DAT,4);

	//GPX4CON和GPX4DAT寄存器直接复用led4
	pmydev->myled5_con = pmydev->myled4_con;
	pmydev->myled5_data = pmydev->myled4_data;

	//设置led四个IO的con寄存器为输出模式
	//比如led2con中先对32位寄存器中的第28位开始清除0
	//然后又最后把整个32位的数据写入进行当前myledX_con的寄存器当中去
	writel((readl(pmydev->myled2_con) & (~(0xF << 28)))| (0x1<<28),pmydev->myled2_con);
	writel((readl(pmydev->myled3_con) & (~(0xF)))| (0x1),pmydev->myled3_con);
	writel((readl(pmydev->myled4_con) & (~(0xF << 16)))| (0x1 << 16),pmydev->myled4_con);
	writel((readl(pmydev->myled5_con) & (~(0xF << 20)))| (0x1 << 20),pmydev->myled5_con);

	//初始化一开始的灯处于熄灭状态
	writel((readl(pmydev->myled2_data) & ~(0x1 << 7)),pmydev->myled2_data);
	writel((readl(pmydev->myled3_data) & ~(0x1 << 0)),pmydev->myled3_data);
	writel((readl(pmydev->myled4_data) & ~(0x1 << 4)),pmydev->myled4_data);
	writel((readl(pmydev->myled5_data) & ~(0x1 << 5)),pmydev->myled5_data);
}

//批量释放映射
void iounmap_ledreg(struct myled_dev *pgmydev)
{
	//释放映射
	iounmap(pmydev->myled2_con);
	pmydev->myled2_con = NULL;
	iounmap(pmydev->myled3_con);
	pmydev->myled3_con = NULL;
	iounmap(pmydev->myled4_con);
	pmydev->myled4_con = NULL;
	iounmap(pmydev->myled2_data);
	pmydev->myled2_data = NULL;
	iounmap(pmydev->myled3_data);
	pmydev->myled3_data = NULL;
	iounmap(pmydev->myled4_data);
	pmydev->myled4_data = NULL;

	pmydev->myled5_data = NULL;
	pmydev->myled5_con = NULL;
}

//写入数据
ssize_t myled_write(struct file *filp, const char __user *usrbuf, size_t size, loff_t *offset)
{
	int bytes = 0;
	printk("---->%s--->%d\n",__func__,__LINE__);

	bytes = copy_from_user(kbuf,usrbuf,4);
	if(bytes > 0){
		printk("copy_from_user failed\n");
		return -1;
	}
	printk("copy_from_user usrbuf:%c\n",kbuf[0]);
	return 0;
}

void led_on(struct myled_dev *pgmydev,int ledno)
{
	switch(ledno)
	{
		case 2:
		//real读取当前数据寄存器的值
		//将第7位为1
			writel(readl(pmydev->myled2_con) | (0x1<<7),pmydev->myled2_data);
			printk("led2 ---->on.\n");
			break;
		case 3:
			writel(readl(pmydev->myled3_con) | (0x1<<0),pmydev->myled3_data);
			printk("led3 ---->on.\n");
			break;
		case 4:
			writel(readl(pmydev->myled4_con) | (0x1<<4),pmydev->myled4_data);
			printk("led4----->on.\n");
			break;
		case 5:
			writel(readl(pmydev->myled5_con) | (0x1<<5),pmydev->myled5_data);
			printk("led4----->on.\n");
			break;
		default:
			printk("led number error\n");
	}

}

void led_off(struct myled_dev *pgmydev,int ledno)
{
	switch(ledno)
	{
		case 2:
		//real读取当前数据寄存器的值
		//将第7位为1
			writel(readl(pmydev->myled2_con) & ~(0x1<<7),pmydev->myled2_data);
			printk("led2 ---->on.\n");
			break;
		case 3:
			writel(readl(pmydev->myled3_con) & ~(0x1<<0),pmydev->myled3_data);
			printk("led3 ---->on.\n");
			break;
		case 4:
			writel(readl(pmydev->myled4_con) & ~(0x1<<4),pmydev->myled4_data);
			printk("led4----->on.\n");
			break;
		case 5:
			writel(readl(pmydev->myled5_con) & ~(0x1<<5),pmydev->myled5_data);
			printk("led5----->on.\n");
			break;
		default:
			printk("led number error\n");	
	}
}


/**
 * myled_ioctl - LED设备的ioctl控制函数
 * @filp: 文件指针，指向打开的设备文件
 * @cmd: ioctl命令码，用于区分不同的控制命令
 * @args: 用户空间传递的参数指针
 *
 * 该函数处理用户空间发送的ioctl命令，用于控制LED设备的开关。
 * 支持FS_LED_ON和FS_LED_OFF两种命令，可以控制LED2-LED5的开关状态。
 * 函数通过copy_from_user从用户空间获取led_desc_t结构体参数，
 * 然后根据命令码和LED编号执行相应的开关操作。
 *
 * 返回值：成功返回0，失败返回错误码
 */
long myled_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	int i,ret;
	printk("---->%s--->%d\n",__func__,__LINE__);

	
	/* 从用户空间复制LED描述符数据到内核空间 */
	ret = copy_from_user(&pmydev->led_f,(void *)args,sizeof(led_desc_t));
	if (ret) {
		printk("copy_from_user failed\n");
		return ret;
	}
		
	/* 根据命令码处理LED开关控制 */
	switch(cmd){
		case FS_LED_ON:
		i = pmydev->led_f.led_num;//获取LED编号
		printk("i= %d\n",i);
			if(i == 2){
				led_on(pmydev,i);
				printk("led2 ---->on.\n");
			}
			else if(i == 3){
				led_on(pmydev,i);
				printk("led3 ---->on.\n");
			}else if(i == 4){
				led_on(pmydev,i);
				printk("led4 ---->on.\n");
			}else if(i == 5){
				led_on(pmydev,i);
				printk("led5 ---->on.\n");
			}
			printk("FS_LED_ON. \n");
			break;
		case FS_LED_OFF:
		i = pmydev->led_f.led_num;
		printk("i= %d\n",i);
			if(i == 2){
				led_off(pmydev,i);
				printk("led2 ---->off.\n");
			}else if(i == 3){
				led_off(pmydev,i);
				printk("led3 ---->off.\n");
			}else if(i == 4){
				led_off(pmydev,i);
				printk("led4 ---->off.\n");
			}else if(i == 5){
				led_off(pmydev,i);
				printk("led5 ---->off.\n");
			}
			printk("FS_LED_OFF. \n");
			break;
		default:
			printk("default :....\n");
			break;
	}

	return 0;
}


int myled_open(struct inode *pnode, struct file *filp)
{
	// 通过container_of宏从inode的cdev成员反向找到myled_dev结构体
	// 并存储到file的private_data，方便后续操作直接获取设备信息
	filp->private_data =(void *) (container_of(pnode->i_cdev,struct myled_dev,mycdev));
	//硬件的初始化工作--收发数据的初始化
	printk("---->%s--->%d\n",__func__,__LINE__);
	return 0;
}

//关闭设备
int myled_close(struct inode *inode, struct file *filp)
{
	
	printk("---->%s--->%d\n",__func__,__LINE__);
	return 0;
}

//设备操作函数
const struct file_operations fops = {
	.open=myled_open,
	.llseek=myled_llseek,
	.read=myled_read,
	.write=myled_write,
	.unlocked_ioctl=myled_ioctl,
	.release=myled_close,
};

//初始化驱动
static int __init myled_init(void)
{
	printk("---->%s--->%d\n",__func__,__LINE__);
	
	/* 注册字符设备驱动，动态分配主设备号 */
	major = register_chrdev(0,name,&fops); 
	if(major <= 0){
		printk("register_chrdev failed!\n");
	}
	printk("register_chrdev success .major: %d\n",major);
	devno = MKDEV(major,0);

		/* 步骤2：分配并初始化设备结构体 */
	pmydev = (struct myled_dev *)kmalloc(sizeof(struct myled_dev),GFP_KERNEL);
	if(NULL == pmydev)  // 内存分配失败
	{
		unregister_chrdev_region(devno,dev_num);  // 释放已申请的设备号
		printk("kmalloc failed\n");  // 打印错误信息
		return -1;  // 初始化失败
	}
	memset(pmydev,0,sizeof(struct myled_dev));  // 初始化内存为0

	//初始化cdev结构体
	cdev_init(&pmydev->mycdev,&fops);
	pmydev->mycdev.owner = THIS_MODULE;//设置cdev结构体的owner字段为当前模块
	//添加设备
	cdev_add(&pmydev->mycdev,devno,dev_num);
	
	/* 创建设备类，用于在/sys/class/目录下创建对应的类目录 */
	pmydev->cls = class_create(THIS_MODULE,name);
	if (!pmydev->cls) {
        kfree(pmydev);            // 回滚：释放内存
        unregister_chrdev_region(devno, dev_num); // 回滚：释放设备号
        printk("class_create failed!\n");
        return -1;
    }
	
	/* 创建设备文件，会在/dev/目录下创建对应的设备节点 */
	pmydev->dev = device_create(pmydev->cls, NULL,MKDEV(major,0),NULL,name);
	 if (!pmydev->dev) {
        class_destroy(pmydev->cls); // 回滚：销毁设备类
        kfree(pmydev);              // 回滚：释放内存
        unregister_chrdev_region(devno, dev_num); // 回滚：释放设备号
        printk("device_create failed!\n");
        return -1;
    }
	printk("class device_create success .\n");

	//硬件初始化
	gpio_ioremap(pmydev);
	
	return 0;
}

//卸载驱动
static void __exit myled_exit(void)
{
	printk("---->%s--->%d\n",__func__,__LINE__);
	
	device_destroy(pmydev->cls,MKDEV(major,0));
	unregister_chrdev(major,name);
	/* 步骤1：解除寄存器映射 */
	iounmap_ledreg(pmydev);

	/* 步骤2：从内核中删除字符设备 */
	cdev_del(&pmydev->mycdev);

	/* 步骤3：释放设备号 */
	unregister_chrdev_region(devno,dev_num);

	/* 步骤4：释放设备结构体内存 */
	kfree(pmydev);
	pmydev = NULL;  // 指针置空，防止野指针
}


module_init(myled_init);
module_exit(myled_exit);
MODULE_LICENSE("GPL");


