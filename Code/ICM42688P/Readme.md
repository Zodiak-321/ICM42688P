# ICM42688P 驱动程序库 - 详细文档

## 📖 目录
1. [项目概述](#项目概述)
2. [核心架构设计](#核心架构设计)
3. [滤波器系统详解](#滤波器系统详解)
4. [中断管理系统详解](#中断管理系统详解)
5. [数据读取与校准](#数据读取与校准)
6. [API参考](#api参考)
7. [使用示例](#使用示例)
8. [调试支持](#调试支持)

## 项目概述

本库是针对TDK InvenSense ICM42688P 6轴IMU的完整驱动程序实现，基于STM32 HAL库开发。代码严格按照ICM42688P技术手册编写，提供了丰富的配置选项和专业的信号处理功能。

## 核心架构设计

### 🏗️ 类层次结构
```
ICM42688 (主类)
├── General: 常规功能管理
│   ├── 传感器使能/禁用
│   ├── 滤波器配置
│   ├── 数据读取
│   └── 偏移校准
├── INT: 中断管理
│   ├── 中断源配置
│   ├── 事件回调注册
│   ├── 优先级调度
│   └── 中断状态管理
├── FIFO: 缓冲区管理（开发中）
└── APEX: 高级算法（开发中）
```

### 🔄 设计理念
1. **模块化设计**：每个功能模块独立，便于维护和扩展
2. **类型安全**：使用强类型枚举，减少配置错误
3. **资源高效**：避免动态内存分配，适合嵌入式系统
4. **调试友好**：提供详细的调试输出和错误信息

## 滤波器系统详解

### 📊 加速度计滤波器
支持两种工作模式和多种滤波器组合：

**LN模式（低噪声）**：
- 支持ODR: 12.5Hz - 32kHz
- 滤波器：UI滤波器 + M2滤波器 + AAF
- UI滤波器阶数：1/2/3阶可选
- 带宽配置：ODR/2 到 ODR/400

**LP模式（低功耗）**：
- 支持ODR: 1.5625Hz - 200Hz
- 滤波器：平均滤波器（1/16次平均）

### 🔄 陀螺仪滤波器
**LN模式**：
- 支持ODR: 12.5Hz - 32kHz
- 滤波器组合：
  - UI滤波器（1/2/3阶）
  - M2滤波器（可选）
  - NF陷波滤波器（可配置中心频率和带宽）
  - AAF抗混叠滤波器

### 🎛️ 预设滤波等级
提供6种预设配置，满足不同应用需求：

| 等级 | UI阶数 | M2滤波 | AAF | 适用场景 |
|------|--------|--------|-----|----------|
| VERY_HIGH | 3阶 | 启用 | 启用 | 高精度测量 |
| HIGH | 3阶 | 启用 | 启用 | 一般IMU应用 |
| MEDIUM | 2阶 | 启用 | 启用 | 平衡性能与功耗 |
| LOW | 1阶 | 禁用 | 启用 | 低延迟应用 |
| VERY_LOW | 1阶 | 禁用 | 启用 | 低功耗模式 |
| VERT_VERY_LOW | 1阶 | 禁用 | 禁用 | 最低功耗 |

### ⚙️ 自动参数计算
库提供智能参数计算功能：
```cpp
// 自动根据ODR计算AAF参数
uint8_t aaf_delt;
uint16_t aaf_deltsqr;
uint8_t aaf_bitshift;
imu.general.find_AFF(ICM42688::General::ACCEL_ODR_1000HZ, 
                     &aaf_delt, &aaf_deltsqr, &aaf_bitshift);

// 自动计算NF参数
uint8_t nf_coswz_sel;
uint16_t nf_coswz;
imu.general.find_NF(2000, &nf_coswz_sel, &nf_coswz);
```

## 中断管理系统详解

### 🎯 事件驱动架构
支持18种中断事件，每种事件都可以独立配置：

```cpp
// 中断事件枚举
enum class IntEvent : uint8_t {
    UI_FSYNC_INT,       // UI帧同步
    PLL_RDY_INT,        // PLL就绪
    RESET_DONE_INT,     // 复位完成
    DATA_RDY_INT,       // 数据就绪（最常用）
    FIFO_THS_INT,       // FIFO阈值
    FIFO_FULL_INT,      // FIFO满
    // ... 共18种事件
};
```

### 📝 回调函数注册
提供灵活的注册机制，支持自定义回调：

```cpp
// 注册数据就绪中断（使用默认回调）
imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                      ICM42688::INT::INT_NUM::INT1);

// 注册自定义回调
imu.Int.register_event(ICM42688::INT::IntEvent::TAP_DET_INT,
                      ICM42688::INT::INT_NUM::INT2,
                      my_tap_callback,
                      &my_context,
                      ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGH);
```

### 🏆 优先级调度系统
中断事件按优先级处理，确保关键事件及时响应：

```cpp
// 设置事件优先级
imu.Int.set_event_priority(ICM42688::INT::IntEvent::DATA_RDY_INT,
                          ICM42688::INT::INT_NUM::INT1,
                          ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGHEST);
```

### 🔧 中断引脚配置
支持灵活的引脚配置：

```cpp
// 配置INT1引脚
imu.Int.set_int1_pin_cfg(GPIOA, GPIO_PIN_0,
                        ICM42688::INT::LEVEL::LEVEL_ACTIVE_HIGH,
                        ICM42688::INT::DRIVE::DRIVE_PUSH_PULL,
                        ICM42688::INT::MODE::LATCHED);

// 配置INT2引脚
imu.Int.set_int2_pin_cfg(GPIOA, GPIO_PIN_1,
                        ICM42688::INT::LEVEL::LEVEL_ACTIVE_LOW,
                        ICM42688::INT::DRIVE::DRIVE_OPEN_DRAIN,
                        ICM42688::INT::MODE::PULSE);
```

## 数据读取与校准

### 📈 数据读取优化
- **智能轴检测**：自动跳过未启用的轴，提高读取效率
- **批量读取**：一次性读取所有数据，减少通信开销
- **类型转换**：自动将原始数据转换为物理量

### ⚖️ 双偏移校准
提供硬件和软件双重偏移校准：

```cpp
// 自动校准（推荐）
imu.general.set_acc_gyro_offset(1000);  // 采样1000次

// 手动设置软件偏移
imu.general._acc_offset_x = -0.1f;
imu.general._gyro_offset_y = 0.05f;
```

### 🧭 安装方向补偿
支持6种安装方向，自动补偿重力影响：

```cpp
// 初始化时指定安装方向
ICM42688 imu(&hi2c1, ICM42688_I2C_ADDR_LOW, 
             ICM42688::ICM42688_MountingOrientation::MOUNT_Z_UP);
```

## API参考

### General类主要方法

```cpp
// 传感器控制
StatusTypeDef enable_acc(...);      // 启用加速度计
StatusTypeDef disable_acc();        // 禁用加速度计
StatusTypeDef enable_gyro(...);     // 启用陀螺仪
StatusTypeDef disable_gyro();       // 禁用陀螺仪
StatusTypeDef enable_temp();        // 启用温度计
StatusTypeDef enable_tmst();        // 启用时间戳

// 数据操作
StatusTypeDef read_data();          // 读取传感器数据
float get_ax();                     // 获取X轴加速度
float get_gy();                     // 获取Y轴角速度
float get_temp();                   // 获取温度

// 校准
StatusTypeDef set_acc_gyro_offset(uint16_t samples);
```

### INT类主要方法

```cpp
// 中断配置
StatusTypeDef set_int1_pin_cfg(...);    // 配置INT1引脚
StatusTypeDef set_int2_pin_cfg(...);    // 配置INT2引脚

// 事件管理
StatusTypeDef register_event(...);      // 注册中断事件
StatusTypeDef unregister_event(...);    // 注销中断事件
StatusTypeDef set_event_priority(...);  // 设置事件优先级
StatusTypeDef set_event_enable(...);    // 启用/禁用事件

// 中断处理
StatusTypeDef process_interrupts(...);  // 处理中断
bool is_event_registered(...);          // 检查事件是否注册
```

## 使用示例

### 示例1：基础数据读取
```cpp
#include "ICM42688.h"

ICM42688 imu(&hi2c1);

void setup() {
    imu.begin();
    
    // 启用传感器
    imu.general.enable_acc();
    imu.general.enable_gyro();
}

void loop() {
    imu.general.read_data();
    
    printf("Acc: %.2f, %.2f, %.2f g\n", 
           imu.general.get_ax(),
           imu.general.get_ay(),
           imu.general.get_az());
    
    printf("Gyro: %.2f, %.2f, %.2f dps\n",
           imu.general.get_gx(),
           imu.general.get_gy(),
           imu.general.get_gz());
    
    HAL_Delay(100);
}
```

### 示例2：中断驱动应用
```cpp
#include "ICM42688.h"

ICM42688 imu(&hi2c1);
volatile bool data_ready = false;

void data_ready_callback(void* context) {
    data_ready = true;
}

void setup() {
    imu.begin();
    
    // 配置传感器
    imu.general.enable_acc(ICM42688::General::ACCEL_ODR_1000HZ,
                          ICM42688::General::ACCEL_FS_16G);
    
    // 注册中断
    imu.Int.set_int1_pin_cfg(GPIOA, GPIO_PIN_0);
    imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                          ICM42688::INT::INT_NUM::INT1,
                          data_ready_callback);
}

// 在EXTI中断服务例程中调用
void EXTI0_IRQHandler() {
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)) {
        imu.Int.process_interrupts(ICM42688::INT::INT_NUM::INT1);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    }
}

void loop() {
    if(data_ready) {
        imu.general.read_data();
        // 处理数据...
        data_ready = false;
    }
}
```

## 调试支持

### 🔍 调试输出
通过定义`ENABLE_DEBUG`启用详细调试信息：

```cpp
#define ENABLE_DEBUG
#include "debug.h"
```

调试信息包括：
- ✅ 操作成功提示（绿色）
- ⚠️ 警告信息（黄色）
- ❌ 错误信息（红色）
- 🔧 处理过程信息（蓝色）

### 📊 寄存器访问跟踪
所有寄存器读写操作都会被记录，便于调试硬件通信问题。

### 🎨 彩色终端输出
支持ANSI颜色代码，使调试信息更易阅读。

---

## 🎯 应用场景

- **无人机飞控**：高频率、低延迟的姿态数据
- **机器人导航**：精确的惯性测量
- **运动捕捉**：高精度运动跟踪
- **健康监测**：步数检测、活动识别
- **工业控制**：振动监测、姿态稳定

## 📝 注意事项

1. **电源稳定性**：确保IMU供电稳定，建议使用LDO
2. **通信速度**：根据ODR选择合适的I2C/SPI时钟频率
3. **中断处理**：中断服务例程应尽量简短
4. **校准环境**：校准时保持IMU静止
5. **温度影响**：温度变化会影响传感器精度，建议定期校准

## 🤝 贡献指南

欢迎提交Issue和Pull Request！对于新功能请求，请先讨论实现方案。

## 📄 许可证

MIT License

---

*本库持续开发中，欢迎反馈和建议！*

