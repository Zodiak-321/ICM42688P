#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "cpp_main.h"

#ifdef HAL_UART_MODULE_ENABLED
#include "usart.h"
#endif

// 定义颜色宏
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// 亮色
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"
#define COLOR_BRIGHT_WHITE   "\033[97m"

// 背景色
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"

// 样式
#define BOLD       "\033[1m"
#define UNDERLINE  "\033[4m"
#define BLINK      "\033[5m"

// TYPE
#define ERROR_TYPE "\033[91m\033[1m[ERROR]\033[0m "
#define WARNING_TYPE "\033[93m\033[1m[WARNING]\033[0m "
#define PROCESS_TYPE "\033[94m\033[1m[PROCESS]\033[0m "
#define OK_TYPE "\033[92m\033[1m[OK]\033[0m "
#define SUCCESS_TYPE "\033[92m\033[1m[SUCCESS]\033[0m "

class DEBUG
{
    public:
        DEBUG(UART_HandleTypeDef* huart) : _huart(huart){}

        UART_HandleTypeDef* _huart = nullptr;

    private:
};

// 写在主函数上面(以下是示例)
// DEBUG debug(&huart3);
// extern "C" int fputc(int ch, FILE *f)
// {
//     HAL_UART_Transmit(debug._huart, (uint8_t *)&ch, 1, 0xFFFF);
//     return ch;
// }

#endif
