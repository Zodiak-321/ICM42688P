#include "src\UserCode\cpp_main\cpp_main.h"

#include "UserCode\ICM42688\ICM42688.h"
#include "UserCode\QMC5883P\qmc5883p.h"
// #include "debug.h"
#include "UserCode\ICM42688\Madgwick.h"
#include "common_data.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "r_adc.h"
#include "r_adc_api.h"
#include "r_agt.h"
#include "r_icu.h"
#include "r_sci_uart.h"
#include "r_timer_api.h"
#include "string.h"
#include "r_iic_master.h"
#include "r_ioport_cfg.h"               // Renesas::Flex Software:Build Configuration


extern "C"{
#include "stdio.h"
}

// sysTick
volatile uint32_t sysTick = 0;

// 传感器相关
ICM42688 icm42688(ICM42688::ICM42688_MountingOrientation::MOUNT_Y_DOWN);
QMC5883P qmc5883p(QMC5883P::QMC5883P_Mode::CONTINUOUS, QMC5883P::QMC5883P_Spd::ODR_100HZ, QMC5883P::QMC5883P_Rng::RNG_12G);
EULER euler(25.98f);

volatile ICM42688::ICM42688_StatusTypeDef res_icm42688;
volatile ICM42688::General::StatusTypeDef res_general_icm42688;
volatile ICM42688::INT::StatusTypeDef res_int_icm42688;
volatile QMC5883P::QMC5883P_Status res_qmc5883p;

void ICM42688_dataRady(void* context)
{
    icm42688.general.read_data();
    qmc5883p.update();
}

// 串口发送完成标志
volatile bool uart1_send_complete_flag = false;
volatile bool uart2_send_complete_flag = false;

// ADC
volatile bool scan_complete_flag = false;  
volatile uint16_t ADC_9_value;
volatile float ADC_PWM_truly_value = 0.6f;

// 主循环定时器
volatile bool main_timer_call = false;
volatile uint16_t counter;

// 数据包空间
uint8_t sendbuf[21] = {0};
uint8_t sendbuf_key1[3] = {0xAF, 0x02, 0xFA};
uint8_t sendbuf_key2[3] = {0xAF, 0x03, 0xFA};

// debug存储空间
uint8_t debug_buf[128] = {0};

// UI状态                                                       
volatile bool isKey1Active = false;              // KEY1激活状态
volatile bool isKey2Active = false;              // KEY2激活状态
volatile bool isKey2Active_triggered_underisTriggerActive = false;
volatile bool isTriggerActive = false;           // 扳机激活状态
volatile int32_t ENCA_count = 0;
volatile float ENCA_speed = 0;
bsp_io_level_t ENCB_state;

void ADC_Read(void)
{
    R_ADC_ScanStart(g_adc0.p_ctrl);
    while(scan_complete_flag == false);
    scan_complete_flag = false;

    R_ADC_Read(g_adc0.p_ctrl, ADC_CHANNEL_9, (uint16_t *)&ADC_9_value);
}

void PWM_Set(uint8_t duty)
{
    if(duty < 0)
        duty = 0;
    if(duty > 100)
        duty = 100;
    timer_info_t info;
    
    R_GPT_InfoGet(g_timer_gpt2.p_ctrl, &info);
    R_GPT_DutyCycleSet(g_timer_gpt2.p_ctrl, (uint32_t)(((uint64_t) info.period_counts * duty) / 100), GPT_IO_PIN_GTIOCA);
}

void ENCA_cacuSpeed(void)
{
    static int32_t last_counter = 0;
    static uint32_t last_time = 0;
    
    ENCA_speed = (float)((ENCA_count - last_counter) * 1000 * 25) / (sysTick - last_time);
    
    last_time = sysTick;
    last_counter = ENCA_count;
}

void ENCA_filter(void)
{
    static float last_ENCA_speed = 0;
    float a = 0.9f;
    
    ENCA_speed = ENCA_speed * a + last_ENCA_speed * (1 - a);
    
    last_ENCA_speed = ENCA_speed;
}

void float_trans_int(float data, uint8_t* out)
{
    union {
        float f;
        uint8_t bytes[4];
    } converter;
    
    converter.f = data;
    
    // STM32是小端架构，需要转换为大端序
    out[0] = converter.bytes[3];  // 最高字节
    out[1] = converter.bytes[2];
    out[2] = converter.bytes[1];
    out[3] = converter.bytes[0];  // 最低字节
}
    
void initializeSensors(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    {
        R_SPI_Open(g_spi1.p_ctrl, g_spi1.p_cfg);
        assert(err == FSP_SUCCESS);
    }
    
    {
        R_IIC_MASTER_Open(g_i2c_master1.p_ctrl, g_i2c_master1.p_cfg);
        assert(err == FSP_SUCCESS);
    }
    
    // 开启外部中断->ICM42688
    {
        err = R_ICU_ExternalIrqOpen(g_external_irq1.p_ctrl, g_external_irq1.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ICU_ExternalIrqEnable(g_external_irq1.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    
    res_qmc5883p = qmc5883p.begin();
    
    res_icm42688 = icm42688.begin();
    
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    res_general_icm42688 = icm42688.general.enable_acc(ICM42688::General::ACCEL_ODR::ACCEL_ODR_50HZ, 
                               ICM42688::General::ACCEL_FS::ACCEL_FS_2G, 
                               ICM42688::General::FILTER_LEVEL::HIGH);

    res_general_icm42688 = icm42688.general.enable_gyro(ICM42688::General::GYRO_ODR::GYRO_ODR_50HZ,
                                ICM42688::General::GYRO_FS::GYRO_FS_250DPS,
                                ICM42688::General::FILTER_LEVEL::HIGH);

//    res_general_icm42688 = icm42688.general.enable_temp();

//    res_general_icm42688 = icm42688.general.enable_tmst(ICM42688::General::TMST_RESOL::TMST_RESOL_1us);

    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    res_general_icm42688 = icm42688.general.set_acc_gyro_offset(500);

    res_int_icm42688 = icm42688.Int.set_int1_pin_cfg(ICM42688::INT::LEVEL::LEVEL_ACTIVE_HIGH,
                                                ICM42688::INT::DRIVE::DRIVE_PUSH_PULL,
                                                ICM42688::INT::MODE::LATCHED);

    res_int_icm42688 = icm42688.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                                                ICM42688::INT::INT_NUM::INT1,
                                                ICM42688_dataRady,
                                                NULL,
                                                ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGH);

    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
}

void initializePeripherals(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    // 开启串口1
    {
        err = R_SCI_UART_Open(g_uart1.p_ctrl, g_uart1.p_cfg);
        assert(err == FSP_SUCCESS);
    }
    // 开启串口2
    {
        err = R_SCI_UART_Open(g_uart2.p_ctrl, g_uart2.p_cfg);
        assert(err == FSP_SUCCESS);
    }
    // 开启外部中断->TRIGGER
    {
        err = R_ICU_ExternalIrqOpen(g_external_irq2.p_ctrl, g_external_irq2.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ICU_ExternalIrqEnable(g_external_irq2.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    // 开启外部中断->KEY1
    {
        err = R_ICU_ExternalIrqOpen(g_external_irq0.p_ctrl, g_external_irq0.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ICU_ExternalIrqEnable(g_external_irq0.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    // 开启外部中断->KEY2
    {
        err = R_ICU_ExternalIrqOpen(g_external_irq9.p_ctrl, g_external_irq9.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ICU_ExternalIrqEnable(g_external_irq9.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    // 开启外部中断->ENCA
    {
        err = R_ICU_ExternalIrqOpen(g_external_irq6.p_ctrl, g_external_irq6.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ICU_ExternalIrqEnable(g_external_irq6.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    // 开启ADC
    {
        err = R_ADC_Open(g_adc0.p_ctrl, g_adc0.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_ADC_ScanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
        assert(err == FSP_SUCCESS);
    }
    // 输出PWM开启
    {
        R_GPT_Open(g_timer_gpt2.p_ctrl, g_timer_gpt2.p_cfg);
        R_GPT_Start(g_timer_gpt2.p_ctrl);
    }
    PWM_Set(0);
}

void initializeTimers(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    // 开启定时器main
    {
        err = R_AGT_Open(g_timer_main.p_ctrl, g_timer_main.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_AGT_Start(g_timer_main.p_ctrl);
        assert(err == FSP_SUCCESS);
        err = R_AGT_Enable(g_timer_main.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
    // 开启定时器systick
    {
        err = R_AGT_Open(g_systick.p_ctrl, g_systick.p_cfg);
        assert(err == FSP_SUCCESS);
        err = R_AGT_Start(g_systick.p_ctrl);
        assert(err == FSP_SUCCESS);
        err = R_AGT_Enable(g_systick.p_ctrl);
        assert(err == FSP_SUCCESS);
    }
}


// ADC中断回调函数
void g_adc_callback(adc_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    scan_complete_flag = true;
}

// 串口1中断回调函数      DEBUG
void g_uart1_callback(uart_callback_args_t *p_args)
{           
    switch(p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            
            break;
        }
        case UART_EVENT_TX_COMPLETE:
        {
            uart1_send_complete_flag = true;
            break;
        }
        default:
            break;
    }
}

// 串口2中断回调函数      HC06
void g_uart2_callback(uart_callback_args_t *p_args)
{
    switch(p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            
            break;
        }
        case UART_EVENT_TX_COMPLETE:
        {
            uart2_send_complete_flag = true;
            break;
        }
        default:
            break;
    }
}

// 定时器中断回调函数main
void g_timer_main_callback(timer_callback_args_t *p_args)
{
    if(p_args->event == TIMER_EVENT_CYCLE_END)
    {
        counter++;

        if(counter == 100)
        {
            ENCA_cacuSpeed();
            ENCA_filter();
            ADC_Read();
            PWM_Set((uint8_t)((((float)(ADC_9_value - 1900) / 4096) * ADC_PWM_truly_value + (1 - ADC_PWM_truly_value)) * 154));
            euler.MadgwickAHRSupdate(icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az(),
                        icm42688.general.get_gx(), icm42688.general.get_gy(), icm42688.general.get_gz(),
                        qmc5883p.getX(), qmc5883p.getY(), qmc5883p.getZ());
//            euler.MadgwickAHRSupdateIMU(icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az(),
//                        icm42688.general.get_gx(), icm42688.general.get_gy(), icm42688.general.get_gz());
            
            counter = 0;
            main_timer_call = true;
        }
    }
}

// 定时器中断回调函数systick
void g_systick_callback(timer_callback_args_t *p_args)
{
    if(p_args->event == TIMER_EVENT_CYCLE_END)
    {
        sysTick += 25;
    }
}

// 外部中断回调函数
void g_external_irq_callback(external_irq_callback_args_t *p_args)
{
    switch(p_args->channel)
    {
        case 0:
        {
            // KEY1
            isKey1Active = true;
            break;
        }
        case 2:
        {
            // TRIGGER
            if(isTriggerActive == false)
                isTriggerActive = true;
            else if(isTriggerActive == true)
                isTriggerActive = false;
            break;
        }
        case 1:
        {
            // ICM42688
            icm42688.Int.process_interrupts(ICM42688::INT::INT_NUM::INT1);
            break;
        }
        case 6:
        {
            // ENCA
            R_IOPORT_PinRead(&g_ioport_ctrl, ENCB, &ENCB_state);
            if(ENCB_state == BSP_IO_LEVEL_HIGH)
                ENCA_count++;
            else if(ENCB_state == BSP_IO_LEVEL_LOW)
                ENCA_count--;
            break;
        }
        case 9:
        {
            // KEY2
            
            
//            if(isTriggerActive == false)
//            {
//                if(isKey2Active == false)
//                    isKey2Active = true;
//                else if(isKey2Active == true)
//                    isKey2Active = false;
//            }
//            else if(isTriggerActive == true)
//            {
//                if(isKey2Active == false && isKey2Active_triggered_underisTriggerActive == false)
//                    isKey2Active = true;
//                else if(isKey2Active == false && isKey2Active_triggered_underisTriggerActive == true)
//                    isKey2Active_triggered_underisTriggerActive = false;
//            }
            
            break;
        }
        default:
            break;
    }
}

void cpp_main(void) 
{
    initializePeripherals();
    initializeSensors();
    initializeTimers();
    
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_HIGH);
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_HIGH);
    PWM_Set(90);
    R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);
    PWM_Set(0);
    R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);
    PWM_Set(90);
    R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);
    PWM_Set(0);
    
    static float pitch_last = 0;
    
    while(1)
    {   
        if(isTriggerActive == true)
        {
            if(main_timer_call == true)
            {
                main_timer_call = false;
                
                sendbuf[0] = 0xAF;
                sendbuf[1] = 0x01;
                float_trans_int(euler.get_pitch() * 0.2f + pitch_last * 0.8f, sendbuf + 2);
                float_trans_int(euler.get_roll(), sendbuf + 6);
                float_trans_int(euler.get_yaw(), sendbuf + 10);
                float_trans_int(ENCA_speed, sendbuf + 14);
                sendbuf[18] = (uint8_t)((ADC_9_value & 0xFF00) >> 8);
                sendbuf[19] = (uint8_t)(ADC_9_value & 0x00FF);
                sendbuf[20] = 0xFA;
                
                
                
//                sprintf((char *)debug_buf, "%f,%f,%f,%f,%f,%f\n", euler.get_pitch() * 0.2 + pitch_last * 0.8, euler.get_roll(), euler.get_yaw(), icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az());
//                R_SCI_UART_Write(g_uart2.p_ctrl, debug_buf, sizeof(debug_buf));
//                while(uart2_send_complete_flag == false);
//                uart2_send_complete_flag = false;
                
                pitch_last = euler.get_pitch();
                
                R_SCI_UART_Write(g_uart2.p_ctrl, sendbuf, 21);
                while(uart2_send_complete_flag == false);
                uart2_send_complete_flag = false;
            }
            else if(isKey1Active == true)
            {
                isKey1Active = false;
                
                R_SCI_UART_Write(g_uart2.p_ctrl, sendbuf_key1, 3);
                while(uart2_send_complete_flag == false);
                uart2_send_complete_flag = false;
            }
            else if(isKey2Active == true)
            {
                isKey2Active = false;
                isKey2Active_triggered_underisTriggerActive = true;
                
                R_SCI_UART_Write(g_uart2.p_ctrl, sendbuf_key2, 3);
                while(uart2_send_complete_flag == false);
                uart2_send_complete_flag = false;
            }
        }
        else if(isTriggerActive == false)
        {
            if(isKey1Active == true)
            {
                isKey1Active = false;
                
                PWM_Set(0);
                R_AGT_Disable(g_timer_main.p_ctrl);
                R_AGT_Disable(g_systick.p_ctrl);
                R_ICU_ExternalIrqDisable(g_external_irq1.p_ctrl);
                
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_LOW);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_HIGH);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_LOW);
                
                res_general_icm42688 = icm42688.general.set_acc_gyro_offset(500);
                
                R_ICU_ExternalIrqEnable(g_external_irq1.p_ctrl);
                R_AGT_Enable(g_timer_main.p_ctrl);
                R_AGT_Enable(g_systick.p_ctrl);
                
                PWM_Set(100);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(0);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(100);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(0);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED1, BSP_IO_LEVEL_HIGH);
            }
            else if(isKey2Active == true)
            {
                PWM_Set(0);
                R_AGT_Disable(g_timer_main.p_ctrl);
                R_AGT_Disable(g_systick.p_ctrl);
                R_ICU_ExternalIrqDisable(g_external_irq1.p_ctrl);
                
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_LOW);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_HIGH);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_LOW);
                
                qmc5883p.calibration(isKey2Active);
                
                R_ICU_ExternalIrqEnable(g_external_irq1.p_ctrl);
                R_AGT_Enable(g_timer_main.p_ctrl);
                R_AGT_Enable(g_systick.p_ctrl);
                
                PWM_Set(100);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(0);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(100);
                R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
                PWM_Set(0);
                R_IOPORT_PinWrite(&g_ioport_ctrl, LED2, BSP_IO_LEVEL_HIGH);
            }
        }
    }
}
