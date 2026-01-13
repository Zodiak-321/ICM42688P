# 简介
QMC5883P的CPP版本,如需C语言版本,可以参考 https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/c

特别感谢 @WilliTourt 为该传感器改写了CPP版本( https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/cpp ),我在他的基础上加入了校准函数以及对接口函数进行了优化

## 使用方法
### 初始化
首先需要明确的一点是,QMC5883P仅支持I2C通信,所以需要确定你STM32确实有开启I2C且在进行HAL库代码生成是有开启代码文件单独生成
<img width="659" height="80" alt="image" src="https://github.com/user-attachments/assets/17d05c3f-4eab-419b-b188-3ac5d4acaf5b" />


## 注意
由于QMC5883P没有专门的物理中断引脚,所以推荐的是将更新数据挂载在另外一个有读取数据完成中断的外设或者以定时器读取
