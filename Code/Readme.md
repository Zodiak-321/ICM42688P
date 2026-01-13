# 简介

文件夹为代码文件夹,存放了主要的代码

其中带#的是对于特殊设备进行特化的代码

## #For Renesas
为瑞萨芯片特化的代码存放处

## ICM42688P
ICM42688P的主要代码,陀螺仪的主体结构

## Madgwick
Madgwick方法解算欧拉角,存在一定Bug,但能用

## QMC5883P
QMC5883P的CPP版本,如需C语言版本,可以参考 https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/c

特别感谢 @WilliTourt 为该传感器改写了CPP版本(https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/cpp),我在他的基础上加入了校准函数以及对接口函数进行了优化

## debug
一个简单的Debug文件,加入后可以对ICM42688P的输出信息进行打印,可以看到设置信息,也有宏定义可以直接关掉,具体可以见ICM的Readme部分

有更好的版本可以参考 https://github.com/WilliTourt/STM32-Elegant-Debug

也可以直接用printf

![94a5182c8e54f5e1c7da55e1a775446b_720](https://github.com/user-attachments/assets/14115864-7588-4dfd-9a3d-4c7d122b6830)

(belike)(原谅我直接使用手机拍的屏幕)
