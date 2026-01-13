# 简介

就是一个很简单的printf重定向以及对一些输出进行了文本美化

## 移植

```c++
DEBUG(UART_HandleTypeDef* huart) : _huart(huart){}
```

可以将参数改成void

```c++
extern "C" int fputc(int ch, FILE *f)
{
    // 修改成你自己的串口发送函数
    HAL_UART_Transmit(debug._huart, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
```

本质上就是printf重定向,没什么好讲的
