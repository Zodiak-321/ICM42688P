#ifndef __IO_H__
#define __IO_H__

#include "hal_data.h"

#define HIGH 1
#define LOW 0

void IO_Write(bsp_io_port_pin_t pin, uint8_t state);
uint8_t IO_Read(bsp_io_port_pin_t pin);

#endif
