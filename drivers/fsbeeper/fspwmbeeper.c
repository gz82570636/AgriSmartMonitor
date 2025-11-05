#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <asm/ioctl.h>
#include "chardev.h"



//蜂鸣器
// 定义GPIO寄存器物理地址（根据硬件手册填写，不同芯片地址不同）
#define GPD0CON 0x114000a0  // GPD0控制器配置寄存器
//定时器模块的基地址
#define TIMER_BASE    0x139D0000             
//定时器模块的配置寄存器,死区调试宽度
#define TCFG0         0x0000                 
//设置二级分频器
#define TCFG1         0x0004                              
//定时器控制寄存器
#define TCON          0x0008               
//递减计数器值
#define TCNTB0        0x000C            
//比较值
#define TCMPB0        0x0010


unsigned int  major;//设备主设备号
unsigned int  dev_num = 1;//设备数量
dev_t devno;//设备号
const char *name = "fsbeeper0";

struct mybeep_dev{ 

	struct cdev mycdev;
	struct class *cls;
	struct device *dev;
	beep_desc_t beeper_f;

	volatile unsigned int *mybeep_con;//蜂鸣器控制寄存器地址
	volatile void *timer_base;//蜂鸣器数据寄存
};
//动态分配内内存
struct mybeep_dev *pmydev = NULL;


char kbuf[] = {'1','2','3','4'};

//移动文件指针
loff_t mybeep_llseek(struct file *filp, loff_t offset, int cnt)
{

	printk("---->%s--->%d\n",__func__,__LINE__);
	return 0;
}

//读取数据
ssize_t mybeep_read(struct file *filp, char __user *usrbuf, size_t count, loff_t *offset)
{
	int bytes = 0;
	printk("---->%s--->%d\n",__func__,__LINE__);

	//模拟数据	
	bytes =	copy_to_user(usrbuf,kbuf,4);
	if(bytes > 0){
		printk("copy_to_user faibeeper!\n");
	}

	return 0;
}
//因为物理地址无法直接访问，需要映射到虚拟地址空间
void gpio_beeper_ioremap(struct mybeep_dev *pgmydev){

    //映射物理地址
	pgmydev->mybeep_con = ioremap(GPD0CON,4);
    //映射定时器模块的基地址
	pgmydev->timer_base = ioremap(TIMER_BASE,4);

	//设置beeperIO的con寄存器为输出模式
    //分频:PCLK/(249 + 1) = 100MHZ/250/4 = 100000 =100kHZ
	//PWM.TCFG0 = 249;
    //0x139D0000 
	writel((readl(pgmydev->mybeep_con) & (~(0xF << 0))) | (0x2<<0),pgmydev->mybeep_con);
    //设置定时器模块的分频系数为250
    //0x139D0000
	writel((readl(pgmydev->timer_base + TCFG0) & (~(0xFF << 0))) | (0xF9 << 0),pgmydev->timer_base + TCFG0);//一级分频250分频
    //0x139D0004,二级分频,4分频 
	writel((readl(pgmydev->timer_base + TCFG1) & (~(0xF << 0))) | (0x2 << 0),pgmydev->timer_base + TCFG1);
    //设置定时器模块的计数值,PWM周期为100MHZ/250/4/100 = 1000
	writel(100,pgmydev->timer_base + TCNTB0);
    //设置定时器模块的比较数值,PWM占空比为80%
	writel(80,pgmydev->timer_base + TCMPB0);

    //自动重装在
	writel((readl(pgmydev->timer_base + TCON)) | (0x1 << 3),pgmydev->timer_base + TCON);
    //关闭反向更新
	writel((readl(pgmydev->timer_base + TCON)) | (0x1 << 1),pgmydev->timer_base + TCON);
    //自动清除
	writel((readl(pgmydev->timer_base + TCON)) & (~(0x1 << 1)),pgmydev->timer_base + TCON);

}

//批量释放映射
void iounmap_beeperreg(struct mybeep_dev *pgmydev)
{
	//释放映射
	iounmap(pmydev->mybeep_con);
	pmydev->mybeep_con = NULL;

    iounmap(pmydev->timer_base);
    pmydev->timer_base = NULL;

}

//写入数据
ssize_t mybeep_write(struct file *filp, const char __user *usrbuf, size_t size, loff_t *offset)
{
	int bytes = 0;
	printk("---->%s--->%d\n",__func__,__LINE__);

	bytes = copy_from_user(kbuf,usrbuf,4);
	if(bytes > 0){
		printk("copy_from_user faibeeper\n");
		return -1;
	}
	printk("copy_from_user usrbuf:%c\n",kbuf[0]);
	return 0;
}

void beeper_on(struct mybeep_dev *pgmydev)
{
    //pwm控制递减计数器开始工作
	writel((readl(pgmydev->timer_base + TCON) | (0x1<<0)),pgmydev->timer_base+TCON);
}

void beeper_off(struct mybeep_dev *pgmydev)
{
    //pwm控制递减计数器停止工作
	writel((readl(pgmydev->timer_base + TCON) & (~(0x1<<0))),pgmydev->timer_base+TCON);
}
static void beeper_freq(struct mybeep_dev *pgmydev,int tcnt,int tcmp)
{
    //pwm周期
    writel(tcnt,pgmydev->timer_base + TCNTB0);
    //pwm设置占空比
    writel(tcmp,pgmydev->timer_base + TCMPB0);
}


/**
 * mybeep_ioctl - beeper设备的ioctl控制函数
 * @filp: 文件指针，指向打开的设备文件
 * @cmd: ioctl命令码，用于区分不同的控制命令
 * @args: 用户空间传递的参数指针
 *
 * 该函数处理用户空间发送的ioctl命令，用于控制beeper设备的开关。
 * 支持FS_beeper_ON和FS_beeper_OFF两种命令，可以控制beeper2-beeper5的开关状态。
 * 函数通过copy_from_user从用户空间获取beeper_desc_t结构体参数，
 * 然后根据命令码和beeper编号执行相应的开关操作。
 *
 * 返回值：成功返回0，失败返回错误码
 */
long mybeep_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	int ret;
	printk("---->%s--->%d\n",__func__,__LINE__);

	
	/* 从用户空间复制beeper描述符数据到内核空间 */
	ret = copy_from_user(&pmydev->beeper_f,(void *)args,sizeof(beep_desc_t));
	if (ret) {
		printk("copy_from_user faibeeper\n");
		return ret;
	}
		
	/* 根据命令码处理beeper开关控制 */
	switch(cmd){
		case BEEP_ON:
				beeper_on(pmydev);
				printk("beeper ---->on.\n");
			break;
		case BEEP_OFF:
				beeper_off(pmydev);
                printk("beeper ---->off.\n");
			break;
        case BEEP_FREQ:
                beeper_freq(pmydev,pmydev->beeper_f.tcnt,pmydev->beeper_f.tcmp);
                printk("beeper ---->set freq.\n");
			break;
		default:
			printk("default :....\n");
			break;
	}

	return -EINVAL;;
}


int mybeep_open(struct inode *pnode, struct file *filp)
{
	// 通过container_of宏从inode的cdev成员反向找到mybeep_dev结构体
	// 并存储到file的private_data，方便后续操作直接获取设备信息
	filp->private_data =(void *) (container_of(pnode->i_cdev,struct mybeep_dev,mycdev));
	beeper_off(pmydev);
	//硬件的初始化工作--收发数据的初始化
	printk("---->%s--->%d\n",__func__,__LINE__);

	return 0;
}

//关闭设备
int mybeep_close(struct inode *inode, struct file *filp)
{
	
	printk("---->%s--->%d\n",__func__,__LINE__);
	beeper_off(pmydev);
	return 0;
}

//设备操作函数
const struct file_operations fops = {
	.open=mybeep_open,
	.llseek=mybeep_llseek,
	.read=mybeep_read,
	.write=mybeep_write,
	.unlocked_ioctl=mybeep_ioctl,
	.release=mybeep_close,
};

//初始化驱动
static int __init mybeep_init(void)
{
	printk("---->%s--->%d\n",__func__,__LINE__);
	
	/* 注册字符设备驱动，动态分配主设备号 */
	major = register_chrdev(0,name,&fops); 
	if(major <= 0){
		printk("register_chrdev faibeeper!\n");
	}
	printk("register_chrdev success .major: %d\n",major);
	devno = MKDEV(major,0);

		/* 步骤2：分配并初始化设备结构体 */
	pmydev = (struct mybeep_dev *)kmalloc(sizeof(struct mybeep_dev),GFP_KERNEL);
	if(NULL == pmydev)  // 内存分配失败
	{
		unregister_chrdev_region(devno,dev_num);  // 释放已申请的设备号
		printk("kmalloc faibeeper\n");  // 打印错误信息
		return -1;  // 初始化失败
	}
	memset(pmydev,0,sizeof(struct mybeep_dev));  // 初始化内存为0

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
        printk("class_create faibeeper!\n");
        return -1;
    }
	
	/* 创建设备文件，会在/dev/目录下创建对应的设备节点 */
	pmydev->dev = device_create(pmydev->cls, NULL,MKDEV(major,0),NULL,"fsbeeper0");
	 if (!pmydev->dev) {
        class_destroy(pmydev->cls); // 回滚：销毁设备类
        kfree(pmydev);              // 回滚：释放内存
        unregister_chrdev_region(devno, dev_num); // 回滚：释放设备号
        printk("device_create faibeeper!\n");
        return -1;
    }
	printk("class device_create success .\n");

	//硬件初始化
	gpio_beeper_ioremap(pmydev);
	
	return 0;
}

//卸载驱动
static void __exit mybeep_exit(void)
{
	printk("---->%s--->%d\n",__func__,__LINE__);
	
	device_destroy(pmydev->cls,MKDEV(major,0));
	unregister_chrdev(major,name);
	/* 步骤1：解除寄存器映射 */
	iounmap_beeperreg(pmydev);

	/* 步骤2：从内核中删除字符设备 */
	cdev_del(&pmydev->mycdev);

	/* 步骤3：释放设备号 */
	unregister_chrdev_region(devno,dev_num);

	/* 步骤4：释放设备结构体内存 */
	kfree(pmydev);
	pmydev = NULL;  // 指针置空，防止野指针
}


module_init(mybeep_init);
module_exit(mybeep_exit);
MODULE_LICENSE("GPL");


