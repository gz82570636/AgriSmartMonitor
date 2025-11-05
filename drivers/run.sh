#!/bin/sh

# 依次加载指定的.ko模块
echo "加载 adc.ko..."
insmod adc.ko 

echo "加载 demoarm.ko..."
insmod demoarm.ko

echo "加载 fspwmbeeper.ko..."
insmod fspwmbeeper.ko

echo "加载 mpu6050_drv.ko..."
insmod mpu6050_drv.ko

echo "所有模块加载命令执行完毕"