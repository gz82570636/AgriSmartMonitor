/*platform driver框架*/
#include <linux/module.h>  
#include <linux/device.h>  
#include <linux/platform_device.h>  
#include <linux/interrupt.h>  
#include <linux/fs.h>  
#include <linux/wait.h>  
#include <linux/sched.h>  
#include <asm/uaccess.h>  
#include <asm/io.h>  

wait_queue_head_t wq;      // 写等待队列，用于阻塞等待数据可读的进程
unsigned int major;//主设备号
struct class *cls = NULL;//类对象
struct device *dev = NULL;//设备对象
dev_t devno;//设备号
const char *adc_name = "fsadc0";
int adc_num  = 1;        // 设备数量（只创建1个总线设备）
static void *fs4412_adc_base;
static struct resource *res1;
static struct resource *res2;

static int have_data = 0;
static int fs4412_adc;//模拟通道数

#define ADCCON 0x0000//控制寄存器
#define ADCDLY 0x0008//延时寄存器
#define ADCDAT 0x000C//数据寄存器
#define CLRINTADC 0x0018//清中断寄存器
#define ADCMUX 0x001C//中断寄存器

int flags = 1;
//#define flags 0

#define DEBUG_PRINTK(msg,DEBUG_FLAG) \
    do{ \
        if(!!DEBUG_FLAG) { \
            printk("---->%s--->%d\n",__func__,__LINE__);\
            printk(msg);\
		}\
    }while(0)

//中断处理函数
static irqreturn_t fs4412_adc_handler(int irqno,void *dev)
{
    have_data = 1;//唤醒条件
    printk("adc--handler\n");
    //清除中断
    writel(0x12,fs4412_adc_base + CLRINTADC);
    //唤醒进程
    wake_up_interruptible(&wq);
    return IRQ_HANDLED;
}
static int fs4412_adc_open(struct inode *inode, struct file *file)  
{  
    DEBUG_PRINTK("fs4412_adc_open",flags); 
    return 0;  
}   
static ssize_t fs4412_adc_read(struct file *file,char __user *buf, size_t len,loff_t *pos)  
{
    //向设备发送读取数据请求
    writel(0x3,fs4412_adc_base + ADCMUX);
    //向设备发送转换请求初始化 
    writel(1 << 0 | 1 << 14 | 0x1<<16| 0xFF <<6 , fs4412_adc_base + ADCCON);

    printk("wait adc complete\n");
    //等待转换完成，如果have_data为1则表示转换完成
    wait_event_interruptible(wq,have_data == 1);
    printk("adc is complete\n");
    //读取数据
    fs4412_adc = readl(fs4412_adc_base + ADCDAT) & 0xfff;

    if(copy_to_user(buf,&fs4412_adc,sizeof(int)))
    {
        return -EFAULT;
    }
    have_data = 0;//重置标志位

    return len;
}


static int fs4412_adc_release(struct inode *inode, struct file *file)  
{
    DEBUG_PRINTK("fs4412_adc_release\n",flags);
    return 0;
}

static struct file_operations  fs4412_adc_ops ={  
    .open    = fs4412_adc_open,  
    .release = fs4412_adc_release,  
    .read    = fs4412_adc_read,  
};  
static int fs4412_adc_probe(struct platform_device *pdev)
{
    int ret;  

    // 1. 从平台设备（pdev）中获取中断资源  
    // platform_get_resource：从平台设备的资源列表中提取指定类型的资源  
    //最终返回的是一个资源结构体指针struct resource
	res1 = platform_get_resource(pdev,IORESOURCE_IRQ, 0);  
    printk("res1.start = 0x%x\n",(unsigned int)res1->start);

    // 2. 从平台设备（pdev）中获取内存资源（硬件寄存器地址范围）  
    // IORESOURCE_MEM 表示内存类资源（通常是硬件寄存器的物理地址范围）  
	res2 = platform_get_resource(pdev,IORESOURCE_MEM, 0);   
    printk("res.start = 0x%d\n",(int)res2->start);
		 
    // 3. 申请中断  
    // request_irq：向内核注册中断处理函数，申请使用指定中断  
    // 参数1：中断号（res1->start 是从资源中获取的中断号）  
	ret = request_irq(res1->start,fs4412_adc_handler,IRQF_DISABLED,"adc1",NULL);  

     // 4. 物理地址映射为内核虚拟地址  
    // ioremap：将硬件寄存器的物理地址映射到内核可访问的虚拟地址（内核不能直接访问物理地址）  
    // 参数1：物理地址起始值（res2->start 是从资源中获取的物理起始地址）  
    // 参数2：映射长度（res2->end - res2->start 是资源的地址范围长度）  
    // 映射后通过 fs4412_adc_base 访问硬件寄存器
    //在这里可以获得物理地址映射出来的虚拟地址
	fs4412_adc_base = ioremap(res2->start,res2->end-res2->start);  

    printk("res1->start :%#x. res2->start :%#x.\n",res1->start,res2->start);
	printk("platform: match ok!\n");

    //自动获取设备号
    major = register_chrdev(0,adc_name,&fs4412_adc_ops);
	if(major <= 0)  // 静态注册失败，尝试动态分配非0
	{
		printk("register_chrdev failed\n");
	}
	printk("register_chrdev success,major=%d\n",major);
	devno = MKDEV(major,0);  // 组合主设备号和次设备号
    
    //初始化等待队列
	init_waitqueue_head(&wq);  

	//自动创建设备类
	cls = class_create(THIS_MODULE,adc_name);
	if(!cls)
	{
		unregister_chrdev_region(devno,adc_num);//释放设备号
		class_destroy(cls);
		printk("class_create error");
		return -1;
	}
	dev = device_create(cls,NULL,devno,NULL,adc_name);
	if(!dev)
	{
		device_destroy(cls,devno);
		class_destroy(cls);
		unregister_chrdev_region(devno,adc_num);//释放设备号
		printk("device_create error");
		return -1;
	}
	printk("class device create success\n");
	return 0;  // 初始化成功
}

static int fs4412_adc_remove(struct platform_device *dev)
{
	printk("platform: driver remove\n");
	return 0;
}

struct platform_device_id testdrv_ids[] = 
{
	[0] = {.name = "fs4412_adc"},
    [1] = {}, //means ending
};

struct of_device_id test_of_ids[] = 
{
	[0] = {.compatible = "fs4412,adc"},//这是一个设备树里面的属性
    [1] = {},
};

struct platform_driver fs4412_adc_driver = {
	.probe = fs4412_adc_probe,
	.remove = fs4412_adc_remove,
	.driver = {
		.name = "fs4412_adc", //必须初始化
        .of_match_table = test_of_ids,
	},
};

static int __init fs4412_adc_init(void)
{
    printk("fs4412_adc_init");  
    //注册平台总线
	return platform_driver_register(&fs4412_adc_driver);
}

static void __exit fs4412_adc_exit(void)
{
    //卸载平台总线
	platform_driver_unregister(&fs4412_adc_driver);
    printk("fs4412_adc_exit");
    return;
}

MODULE_LICENSE("GPL");
module_init(fs4412_adc_init);
module_exit(fs4412_adc_exit);