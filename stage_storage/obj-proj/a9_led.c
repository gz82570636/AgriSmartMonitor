#include <stdio.h> 
#include "cgic.h" 
#include <string.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define N 8

/*
 * 消息结构体定义
 * 用于在进程间通信中传递消息数据
 * 
 * 成员说明:
 * type:    消息队列类型标识符，用于区分不同的消息队列
 * msgtype: 消息类型标识符，用于区分同一消息队列中的不同消息类型
 * text:    消息正文内容，长度为N的无符号字符数组
 */
struct msg
{
	long type;
	long msgtype;
	unsigned char text[N];
};



/**
 * @brief CGI主函数，处理LED控制请求并发送消息到消息队列
 * 
 * 该函数通过CGI接口接收前端传入的LED控制参数和店铺编号，
 * 构造消息并通过System V消息队列发送给后端处理程序。
 * 同时返回HTML页面提示发送成功，并自动跳转回原页面。
 * 
 * @return int 返回0表示执行成功
 */
int cgiMain() 
{ 
	key_t key;
	char buf[N];//存储接收到的消息
	char sto_no[2];//店铺编号
	int msgid;
	struct msg msg_buf;

	// 初始化消息结构体
	memset(&msg_buf,0,sizeof(msg_buf));
	
	// 获取前端传递的LED状态和店铺编号参数
	cgiFormString("led",buf,N);//LED状态
	cgiFormString("store",sto_no,2);//店铺编号

	// 获取System V IPC键值
	if((key = ftok("/tmp", 'g')) < 0)
	{
		perror("ftok");
		exit(1);
	}

	// 获取消息队列标识符
	if((msgid = msgget(key, 0666)) < 0)
	{
		perror("msgget");
		exit(1);
	}

	// 清空消息文本内容
	bzero (msg_buf.text, sizeof (msg_buf.text));

	// 根据LED状态构造控制命令字节
	// 命令格式：[店铺号(2位)][保留位(2位)][LED状态(1位)]
	if (buf[0] == '1')
	{
		// LED开启：将店铺号左移6位，LED状态位置1
		//01000001
		msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (0x0 << 4) | (1 << 0);
	}
	else
	{
		// LED关闭：将店铺号左移6位，LED状态位置0
		msg_buf.text[0] = ((sto_no[0] - 48)) << 6 | (0x0 << 4) | (0 << 0);
	}

	// 设置消息类型并发送消息到队列
	msg_buf.type = 1L;
	msg_buf.msgtype = 1L;
	// 发送消息到消息队列
	msgsnd(msgid, &msg_buf,sizeof(msg_buf)-sizeof(long),0);

	// 转换店铺编号为数值形式
	sto_no[0] -= 48;

	// 输出HTML响应页面
	cgiHeaderContentType("text/html\n\n"); 
	fprintf(cgiOut, "<HTML><HEAD>\n"); 
	fprintf(cgiOut, "<TITLE>My CGI</TITLE></HEAD>\n"); 
	fprintf(cgiOut, "<BODY>"); 

	fprintf(cgiOut, "<H2>send sucess</H2>");

	// 设置页面自动跳转回对应的店铺控制页面
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"1;url=../a9_zigbee%d.html\">", sto_no[0]);
	fprintf(cgiOut, "</BODY>\n"); 
	fprintf(cgiOut, "</HTML>\n"); 


	return 0; 
} 
