#ifndef __IIC_H__
#define __IIC_H__

#include "io.h"

typedef struct {
	bsp_io_port_pin_t SCL_Pin;
	bsp_io_port_pin_t SDA_Pin;
	uint8_t delay_us;
} IIC_BusCfg;

void IIC_Init_Soft(bsp_io_port_pin_t SCL, bsp_io_port_pin_t SDA, uint8_t delay_us, uint8_t bus);
void IIC_Start_Soft(uint8_t bus);
void IIC_Stop_Soft(uint8_t bus);
void IIC_SendByte_Soft(uint8_t bus, uint8_t byte);
uint8_t IIC_ReadByte_Soft(uint8_t bus);
uint8_t IIC_RcvACK_Soft(uint8_t bus);
void IIC_SendACK_Soft(uint8_t bus, uint8_t ack);


#endif
