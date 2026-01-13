# 简介

[中文](README.md) | [English](Readme_en.md)

本仓库是基于C++写的ICM42688P库,封装完整,使用方便

并配套了单独的Madgwick解算欧拉角以供参考,如果你手上还有QMC5883P磁力计,也有相应的库可以直接使用以达成九轴解算欧拉角

此为本仓库主要为STM32做的,但是移植方便,将主要需要修改的函数都放在了最前面,例如Code里面就有打瑞萨杯的时候对磁力计和陀螺仪代码所做的特化,仅供参考

# 特点

一个专为ICM42688P 6轴IMU（3轴加速度计 + 3轴陀螺仪）设计的C++驱动程序库，适用于STM32平台，提供灵活配置和强大的功能。

## ✨ 主要特点
### 🔧 灵活配置的滤波器系统
多级滤波器可配置：支持UI滤波器（1/2/3阶）、M2滤波器、AAF（抗混叠滤波器）、NF（陷波滤波器）

自动参数计算：自动根据ODR计算AAF和NF的最佳参数

预设滤波等级：提供VERY_HIGH到VERY_LOW等多种预设模式

### 🚀 智能中断管理系统
事件回调机制：支持18种中断事件，可注册自定义回调函数

优先级调度：中断事件按优先级处理，确保关键事件及时响应

灵活注册：支持INT1、INT2独立或同时配置不同回调

### 🔌 双通信接口支持
即插即用：同时支持I2C和SPI，通过条件编译自动选择

错误处理：完善的通信错误检测和调试输出

### 🎯 专业级功能
软件/硬件双偏移校准：自动补偿传感器偏移

安装方向补偿：支持6种不同安装方向

时间戳支持：1μs/16μs两种分辨率

实时数据读取：优化的数据读取机制

### 🚀 快速开始
```cpp
// 初始化IMU
ICM42688 imu(&hi2c1);  // I2C方式
// ICm42688 imu(&hspi1, GPIOA, GPIO_PIN_4);  // SPI方式

// 初始化设备
imu.begin();

// 启用加速度计和陀螺仪（预设滤波等级）
imu.general.enable_acc(ICM42688::General::ACCEL_ODR_200HZ, 
                       ICM42688::General::ACCEL_FS_2G,
                       ICM42688::General::FILTER_LEVEL::MEDIUM);
imu.general.enable_gyro(ICM42688::General::GYRO_ODR_200HZ,
                        ICM42688::General::GYRO_FS_500DPS,
                        ICM42688::General::FILTER_LEVEL::MEDIUM);

// 注册数据就绪中断
imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT, 
                      ICM42688::INT::INT_NUM::INT1, 
                      my_data_callback);

// 读取数据
imu.general.read_data();
float ax = imu.general.get_ax();
```
### 📁 代码结构
```text
ICM42688.h/cpp          # 主类，通信接口
├── General            # 常规功能：传感器配置、数据读取
├── INT                # 中断管理：事件注册、优先级处理
├── FIFO              # FIFO缓冲区管理（开发中）
└── APEX              # 高级功能：计步器、敲击检测等（开发中）
```
### 🔧 系统要求
STM32 HAL库

C++11或更高版本

I2C或SPI外设


# 仓库结构

Code 作为主要代码的存放处

Document 存放主要的参照文件

# 未来方向

[26/01/13]可能会考虑将FIFO和APEX部分完成,因为目前是为无人机做的这个项目,对于FIFO和APEX的要求不是很高

# 使用方法

详细请移步到具体的部分下的Readme.md
