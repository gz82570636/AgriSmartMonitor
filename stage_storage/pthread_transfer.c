#include "data_global.h"
#include <string.h>
#include "chardev.h"
#include <math.h>
#include "linuxuart.h"

//定义文件描述符
int adc_fd;
int mpu_fd;
int DH11_fd;

extern pthread_cond_t cond_transfer;
extern pthread_mutex_t mutex_transfer;
extern struct env_info_client_addr sm_all_env_info;

float dh_temp,dh_humidity;

extern int temMAX,temMIN,humMAX,humMIN;

char DH11_buf[4] = {0};
char dh11_data[5] = {0};
char temp[3] = {0};
char hum[3] = {0};

int file_env_info_a9_zigbee(struct env_info_client_addr *rt_status,int home_id);

//这个线程的作用是把
/*
真实数据获取:transfer线程
1、驱动正常工作
2、要吃采集数据OK
3、将应用层的open read
write ioctl close 过程放到线程当中
4、通过read或ioctl获取驱动的数据井填充到结构体当中
===>交给refresh刷新线程将数据刷新
到网页上
*/
void *pthread_transfer(void *arg)
{ 
    int home_id = 1;
    adc_fd = open(ADC_DEV,O_RDWR);
    mpu_fd = open(MPU6050_DEV,O_RDWR);
    DH11_fd = open_port(ZIGBEE_DEV);
    if(adc_fd < 0 || mpu_fd < 0 || DH11_fd < 0)
    {
        printf("open adc and mpu6050 and DH11 error\n");
    }
    set_com_config(DH11_fd,115200,8,'N',1); 

    while(1)
    {
        //上锁
        pthread_mutex_lock(&mutex_transfer);
        pthread_cond_wait(&cond_transfer,&mutex_transfer);
        //printf("begin transfer\n");

        //把数据填充到sm_all_env_info这个共享的结构体里面
        file_env_info_a9_zigbee(&sm_all_env_info,home_id);

        sleep(1);
    
        pthread_mutex_unlock(&mutex_transfer);
    }
    close(adc_fd);
    close(mpu_fd);
    close(DH11_fd);

}

int file_env_info_a9_zigbee(struct env_info_client_addr *rt_status,int home_id)
{ 
    int env_info_size = sizeof(struct env_info_client_addr);
    //printf("env_info_size = %d\n",env_info_size);
    //用来获取read读取到的字符数
    int read_len;
    char *space_pos;        // 用于存储空格的位置

    //定义超过时间1s的文件描述符集合
    struct timeval tv;
    //定义可读集合
    fd_set readfds;
    //定义select返回值
    int select_ret;


    rt_status->monitor_no[home_id].zigbee_info.head[0] = 'm';
    rt_status->monitor_no[home_id].zigbee_info.head[1] = 's';
    rt_status->monitor_no[home_id].zigbee_info.head[2] = 'm';
    rt_status->monitor_no[home_id].zigbee_info.head[3] = 'z';

    //temp读取zigbee温度数据
    strcpy(DH11_buf,"22\n");
    //printf("DH11_buf = %s\n",DH11_buf);
    write(DH11_fd,DH11_buf,strlen(DH11_buf)); 

    FD_ZERO(&readfds);
    FD_SET(DH11_fd,&readfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    //监视文件描述符DH11_fd是否有可读数据
    select_ret = select(DH11_fd + 1,&readfds,NULL,NULL,&tv);
    if(select_ret == -1)
    {
        perror("select");
    }else if(select_ret == 0)
    {
        //printf("without read anything , use old\n");
        //把老的数据上传
        rt_status->monitor_no[home_id].zigbee_info.temperature = dh_temp;
        rt_status->monitor_no[home_id].zigbee_info.humidity = dh_humidity;
    }else{
        read_len = read(DH11_fd,dh11_data,sizeof(dh11_data));
        if(read_len > 0)
        {
            //printf("read DH11 success\n");
            dh11_data[read_len] = '\0';
            strncpy(temp,dh11_data,2);
            temp[2] = '\0';
            strncpy(hum,dh11_data + 2,2);
            hum[2] = '\0';
            dh_temp = (float)atof(temp);
            dh_humidity = (float)atof(hum);
            //printf("dh11_temp = %f\n",dh_temp);
            //printf("dh11_data = %f\n",dh_humidity);
            rt_status->monitor_no[home_id].zigbee_info.temperature = dh_temp;
            rt_status->monitor_no[home_id].zigbee_info.humidity = dh_humidity;
        }else{
            printf("read is failed\n");
        }
    }
    rt_status->monitor_no[home_id].zigbee_info.humidityMAX = humMAX;
    rt_status->monitor_no[home_id].zigbee_info.humidityMIN = humMIN;
    rt_status->monitor_no[home_id].zigbee_info.tempMAX = temMAX;
    rt_status->monitor_no[home_id].zigbee_info.tempMIN = temMIN;


    //获取真实数据
    int adc_sensor_data;
    union mpu6050_data data;

     // 定义专用变量接收换算后的数据
    float accel_x, accel_y, accel_z; // 加速度（单位：g）
    float gyro_x, gyro_y, gyro_z;    // 角速度（单位：°/s）
    float temp_c;                    // 温度（单位：°C）
    float adc_c;                    // adc 数据

		

    read(adc_fd, &adc_sensor_data,4);//读取adc数据,从创建的驱动中
    adc_c = (1.8*adc_sensor_data)/4096;
	//printf("adc value :%0.2fV.\n",adc_c);  
    // 计算原始ADC值
    //float adc_raw = (1.8 * adc_sensor_data) / 4096;
    // 四舍五入保留两位小数（乘以100→四舍五入→除以100）

    //获取mpu6050数据
    ioctl(mpu_fd,GET_GYRO,&data);
    //printf("gyro: x = %05d  y = %05d z = %05d\n",data.gyro.x,data.gyro.y,data.gyro.z);
    gyro_x = (float)data.gyro.x / 16.4;
	gyro_y = (float)data.gyro.y / 16.4;
	gyro_z = (float)data.gyro.z / 16.4;
	//printf("gyro : x=%.1f °/s, y=%.1f °/s, z=%.1f °/s\n", gyro_x, gyro_y, gyro_z);
	
    ioctl(mpu_fd,GET_ACCEL,&data);
    accel_x = (float)data.accel.x / 16384.0;
	accel_y = (float)data.accel.y / 16384.0;
	accel_z = (float)data.accel.z / 16384.0;
	//printf("accel data: x = %05d, y = %05d, z = %05d\n", data.accel.x,data.accel.y,data.accel.z);
	//printf("accel: x=%.3f g, y=%.3f g, z=%.3f g\n", accel_x, accel_y, accel_z);

    ioctl(mpu_fd,GET_TEMP,&data);
    temp_c = (float)data.temp / 340.0 + 36.53;
    //printf("temp: %d\n",data.temp);
	//printf("temp_c data: %.1f °C\n", temp_c);

    //把数据放到共享结构体中供refresh去获取
   	rt_status->monitor_no[home_id].a9_info.head[0]  = 'm';
	rt_status->monitor_no[home_id].a9_info.head[1]  = 's';
	rt_status->monitor_no[home_id].a9_info.head[2]  = 'm';
	rt_status->monitor_no[home_id].a9_info.head[3]  = 'a'; 

    //然后把adc数据上传到这个共享结构体当中供refresh去获取
    rt_status->monitor_no[home_id].a9_info.adc = adc_c;
    //rt_status->monitor_no[home_id].a9_info.adc = roundf(adc_raw * 100) / 100;

    rt_status->monitor_no[home_id].a9_info.gyrox = gyro_x;
    rt_status->monitor_no[home_id].a9_info.gyroy = gyro_y;
    rt_status->monitor_no[home_id].a9_info.gyroz = gyro_z;

    rt_status->monitor_no[home_id].a9_info.aacx = accel_x;
    rt_status->monitor_no[home_id].a9_info.aacy = accel_y;
    rt_status->monitor_no[home_id].a9_info.aacz = accel_z;

    rt_status->monitor_no[home_id].a9_info.temp = temp_c;

    return 0;
}