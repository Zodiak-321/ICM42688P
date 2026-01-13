# 简介
Madgwick方法解算欧拉角,存在一定Bug,但能用

函数主体是来自一位知乎博主,但是我现在已经找不到了,如果该大佬要求删改,请联系我

本质上是先利用三轴加速度解算出一个模糊的角度,再用这个角度去修正陀螺仪和磁力计,再进行数据融合得到角度

全程是使用四元数计算,所以该文件函数也可以得到四元数

缺点在于由于本质是先用加速度去修正的,所以对来自加速度计的加速度质量要求非常高.且天然的缺陷就是该方法只适用于静止下的角度获取,
当机体有高加速度时,会极大影响加速度的归一化,导致初始模糊角度十分不正常

所以还是推荐其他例如卡尔曼滤波(也许可以)去解决这个问题

## 移植到你的平台

```c++
uint32_t EULER::getTick(void)
{
	// return HAL_GetTick();
    // 你的获取系统时钟函数,单位是ms
}
```

## 使用方法
存在两个解算版本,一个是仅陀螺仪六轴解算(MadgwickAHRSupdateIMU),另一个是陀螺仪+磁力计的九轴解算欧拉角(MadgwickAHRSupdate)

九轴解算几乎不会有任何yaw轴的漂移,六轴会有极小的漂移(实测约每分钟0.3°)

1. 在你需要的地方初始化EULER类
```c++
EULER euler(0.88f);
```

其中里面的值为融合时所需要的参数,建议是在0.8-0.98之间,但是需要与实际结合,例如我在Renesas的工程里面就写的25.25,十分夸张

2. 在合适的地方进行欧拉角解算

例如:

```c++
void test(void* context)
{
    ICM42688* icm = static_cast<ICM42688*>(context);
    icm->general.read_data();
    euler.MadgwickAHRSupdateIMU(icm->general.get_ax(), icm->general.get_ay(), icm->general.get_az(),
                                 icm->general.get_gx(), icm->general.get_gy(), icm->general.get_gz());
    // printf("%.3f,%.3f,%.3f\n", euler.get_pitch(), euler.get_roll(), euler.get_yaw());
    // printf("%f,%f,%f,%f\n", euler.get_q0(), euler.get_q1(), euler.get_q2(), euler.get_q3());
}
```

或例如:

```c++
euler.MadgwickAHRSupdate(icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az(),
            icm42688.general.get_gx(), icm42688.general.get_gy(), icm42688.general.get_gz(),
            qmc5883p.getX(), qmc5883p.getY(), qmc5883p.getZ());
'''

3. 使用欧拉角或者四元数

```c++
// 四元数
q0 = get_q0();
q1 = get_q1();
q2 = get_q2();
q3 = get_q3();

// 欧拉角(单位为°)
pitch = get_pitch();
roll = get_roll(); 
yaw = get_yaw();  
```

## 注意
本方法存在Bug,如代码中注释部分,不知道为什么角速度需要额外乘以一个2 * PI(我已经检查过输入输出单位了,但不排除检查失误)

如果你有能力和时间的话或许你可以帮帮我,但也可以就这么错下去,因为不影响实际输出
