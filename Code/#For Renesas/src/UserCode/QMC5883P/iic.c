/*
The "software IIC" part of this IIC library is partly transplanted from my STC89C52RC microcontroller library.
("////": Potential risky codes that may cause timing failures or other unintended consequences.)
*/

#include "iic.h"
#include "io.h"
#include "stdio.h"

static IIC_BusCfg IIC_Bus[8];

/**
	* @brief  软件IIC初始化函数
	* @param  SCL SCL引脚
	* @param  SDA SDA引脚
	* @param  delay_us IIC速度，此值为每次读写后的延时，单位为us
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @retval None
	*/
void IIC_Init_Soft(bsp_io_port_pin_t SCL, bsp_io_port_pin_t SDA, uint8_t delay_us, uint8_t bus) {

	IIC_Bus[bus].SCL_Pin = SCL;
	IIC_Bus[bus].SDA_Pin = SDA;
	IIC_Bus[bus].delay_us = delay_us;

	IO_Write(SCL, HIGH);
	IO_Write(SDA, HIGH);
}

void wSCL(uint8_t bus, uint8_t state) {
	IO_Write(IIC_Bus[bus].SCL_Pin, state);
	if (IIC_Bus[bus].delay_us > 0) {
		R_BSP_SoftwareDelay(IIC_Bus[bus].delay_us, BSP_DELAY_UNITS_MICROSECONDS);
	}
}

void wSDA(uint8_t bus, uint8_t state) {
	IO_Write(IIC_Bus[bus].SDA_Pin, state);
	if (IIC_Bus[bus].delay_us > 0) {
		R_BSP_SoftwareDelay(IIC_Bus[bus].delay_us, BSP_DELAY_UNITS_MICROSECONDS);
	}
}

uint8_t rSDA(uint8_t bus) {
	uint8_t state = IO_Read(IIC_Bus[bus].SDA_Pin);
	if (IIC_Bus[bus].delay_us > 0) {
		R_BSP_SoftwareDelay(IIC_Bus[bus].delay_us, BSP_DELAY_UNITS_MICROSECONDS);
	}
	return state;
}

/**
	* @brief  软件IIC开始信号
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @retval None
	*/
void IIC_Start_Soft(uint8_t bus) {
	wSDA(bus, HIGH);
	wSCL(bus, HIGH);

	wSDA(bus, LOW);
	wSCL(bus, LOW);
}

/**
	* @brief  软件IIC结束信号
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @retval None
	*/
void IIC_Stop_Soft(uint8_t bus) {
	wSDA(bus, LOW);
	wSCL(bus, HIGH);

	wSDA(bus, HIGH);
}

/**
	* @brief  软件IIC发送一个字节
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @param  Byte 需要发送的字节
	* @retval None
	*/
void IIC_SendByte_Soft(uint8_t bus, uint8_t byte) {
	for (int i = 0; i < 8; i++) {
		// wSDA(bus, (byte & 0x80) >> 7);
		// byte <<= 1;
		// wSCL(bus, HIGH);
		// wSCL(bus, LOW);

		wSDA(bus, byte & (0x80 >> i));
		wSCL(bus, HIGH);
		wSCL(bus, LOW);
	}

	//// // 额外的一个时钟，不处理应答信号
	//// wSCL(bus, HIGH);
	//// wSCL(bus, LOW);
}

/**
	* @brief  软件IIC读取一个字节
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @retval byte 获取到的字节
	*/
uint8_t IIC_ReadByte_Soft(uint8_t bus) {
	uint8_t byte = 0x00;
	uint8_t i;
	// wSCL(bus, LOW); ////
	wSDA(bus, HIGH);

	for (i = 0; i < 8; i++) {
		wSCL(bus, HIGH);
		if (rSDA(bus) == HIGH) {
			byte |= (0x80 >> i);
		}
		wSCL(bus, LOW);
	}

	return byte;
}

/**
	* @brief  软件IIC接收应答
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @retval ack, 0应答, 1不应答
	*/
uint8_t IIC_RcvACK_Soft(uint8_t bus) {
	uint8_t ack;
	wSCL(bus, LOW); ////
	wSDA(bus, HIGH);

	wSCL(bus, HIGH);
	ack = rSDA(bus);
	wSCL(bus, LOW);

	return ack;
}

/**
	* @brief  软件IIC发送应答
	* @param  bus 当前操作的软件IIC总线号，范围 0-7
	* @param  ack 应答信息, 0应答, 1不应答
	* @retval None
	*/
void IIC_SendACK_Soft(uint8_t bus, uint8_t ack) {
	wSCL(bus, LOW); ////
	wSDA(bus, ack);
	wSCL(bus, HIGH);
	wSCL(bus, LOW);
}
