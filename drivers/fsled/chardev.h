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


#endif
