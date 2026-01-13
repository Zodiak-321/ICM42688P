#include "io.h"

void IO_Write(bsp_io_port_pin_t pin, uint8_t state) {
    if (state != LOW) {
        R_IOPORT_PinWrite(g_ioport.p_ctrl, pin, BSP_IO_LEVEL_HIGH);
    } else {
        R_IOPORT_PinWrite(g_ioport.p_ctrl, pin, BSP_IO_LEVEL_LOW);
    }
}

uint8_t IO_Read(bsp_io_port_pin_t pin) {
    bsp_io_level_t * p_pin_value;
    R_IOPORT_PinRead(g_ioport.p_ctrl, pin, p_pin_value);
    if (*p_pin_value == BSP_IO_LEVEL_HIGH) {
        return HIGH;
    }
    return LOW;
}
