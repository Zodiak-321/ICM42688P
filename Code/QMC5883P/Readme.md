# 简介
QMC5883P的CPP版本,如需C语言版本,可以参考 https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/c

特别感谢 @WilliTourt 为该传感器改写了CPP版本( https://github.com/Zodiak-321/Neigungssensor/tree/main/Sensors/QMC5883P/cpp ),我在他的基础上加入了校准函数以及对接口函数进行了优化

## 使用方法
'''CPP
printf();
'''

## 注意
由于QMC5883P没有专门的物理中断引脚,所以推荐的是将更新数据挂载在另外一个有读取数据完成中断的外设或者以定时器读取
