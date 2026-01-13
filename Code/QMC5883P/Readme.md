# 简介
QMC5883P的CPP版本,如需C语言版本,可以参考 https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/c

特别感谢 @WilliTourt 为该传感器改写了CPP版本( https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/cpp ),我在他的基础上加入了校准函数以及对接口函数进行了优化

## 使用方法
首先需要明确的一点是,QMC5883P仅支持I2C通信,所以需要确定你STM32确实有开启I2C且在进行HAL库代码生成是有开启代码文件单独生成

<img width="659" height="80" alt="image" src="https://github.com/user-attachments/assets/17d05c3f-4eab-419b-b188-3ac5d4acaf5b" />

1. 在cpp_main.cpp(或者是你需要使用的地方)进行类的初始化

```c++
QMC5883P qmc5883p(&hi2c1, QMC5883P::QMC5883P_Mode::NORMAL);
```

原型有一些default

```c++
QMC5883P(I2C_HandleTypeDef* hi2c, QMC5883P_Mode mode, QMC5883P_Spd spd = QMC5883P_Spd::ODR_50HZ,
       QMC5883P_Rng rng = QMC5883P_Rng::RNG_2G);
```

对于其中的一些可选项,详细可以仔细看看.h中的一些定义

2. 在cpp_main.cpp的最开始进行begin

```c++
res_qmc5883p = qmc5883p.begin();
```

3. 在需要的地方(强烈建议是中断或定时器中断中)进行数据更新

```c++
qmc5883p.update();
```

内置了数据是否准备好判断,所以无需担心无效数据读取带来的性能降低

4. 使用数据

```c++
magX = qmc5883p.getX();
magY = qmc5883p.getY();
magZ = qmc5883p.getZ();
```

返回数据为float类型

## 注意
1. 由于QMC5883P没有专门的物理中断引脚,所以推荐的是将更新数据挂载在另外一个有读取数据完成中断的外设或者以定时器读取
   
2. 如果你是在你的环境第一次使用磁力计,强烈建议你进行磁校准

调用以下函数

```c++
qmc5883p.calibration(true);
```

并在空中以'8'字型匀速慢晃动进行硬铁和软铁校准

以更新数值并填入.h文件

```c++
// Magnetic calibration parameters
#define MAG_X_OFFSET            0.167200f
#define MAG_Y_OFFSET            -0.906200f
#define MAG_Z_OFFSET            -0.713000f
#define MAG_X_SCALE             2.052229f
#define MAG_Y_SCALE             0.711101f
#define MAG_Z_SCALE             0.903787f
```

非常重要,直接关系到你的磁力计是否精准

如果你不想填入这里永久保存,那也可以保证自己每次启动MCU时都进行一次'8'字校准,获取值也会自动更新到MCU中,只不过断电不会保留
