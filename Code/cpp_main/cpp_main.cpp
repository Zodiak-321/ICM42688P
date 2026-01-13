#include "cpp_main.h"

#include "ICM42688.h"
#include "debug.h"
#include "Madgwick.h"
#include "qmc5883p.h"

int16_t ax;
ICM42688::ICM42688_StatusTypeDef res_icm42688;
ICM42688::General::StatusTypeDef res_general;
ICM42688::INT::StatusTypeDef res_Int;

uint8_t data_ready;
uint8_t data_demo[50];

ICM42688 icm42688(&hspi2, CS_GPIO_Port, CS_Pin);
// ICM42688 icm42688(&hi2c1, 0x68);
EULER euler(0.88f);

DEBUG debug(&huart3);
extern "C" int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(debug._huart, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

void test(void* context)
{
    ICM42688* icm = static_cast<ICM42688*>(context);
    icm->general.read_data();
    euler.MadgwickAHRSupdateIMU(icm->general.get_ax(), icm->general.get_ay(), icm->general.get_az(),
                                 icm->general.get_gx(), icm->general.get_gy(), icm->general.get_gz());

    data_ready = 1;
    // printf("%.3f,%.3f,%.3f\n", euler.get_pitch(), euler.get_roll(), euler.get_yaw());
    // printf("%f,%f,%f,%f\n", euler.get_q0(), euler.get_q1(), euler.get_q2(), euler.get_q3());
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    icm42688.Int.process_interrupts(static_cast<ICM42688::INT::INT_NUM>(icm42688.get_GPIO_PinNum(GPIO_Pin)));
}

void float_trans_int(float data, uint8_t* out)
{
    *out = (uint8_t)(((uint32_t)(data * 10000) & 0xFF000000) >> 24);
    *(out + 1) = (uint8_t)(((uint32_t)(data * 10000) & 0x00FF0000) >> 16);
    *(out + 2) = (uint8_t)(((uint32_t)(data * 10000) & 0x0000FF00) >> 8);
    *(out + 3) = (uint8_t)((uint32_t)(data * 10000) & 0x000000FF);
}

// void int_trans_float(uint8_t* data, float* out)
// {
//     *out = ((float)(((uint32_t)*data) << 24 | ((uint32_t)*(data + 1)) << 16 | ((uint16_t)*(data + 2)) << 8 | *(data + 3))) / 10000.0f;
// }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim4)
    {
        if(data_ready == 1)
        {
            data_ready = 0;
            data_demo[0] = 0xAA;
            float_trans_int(euler.get_pitch(), data_demo + 1);
            float_trans_int(euler.get_roll(), data_demo + 5);
            float_trans_int(euler.get_yaw(), data_demo + 9);
            data_demo[13] = 0xFF;

            HAL_UART_Transmit(&huart5, data_demo, 14, 0xFFFF);
        } 
    }
}

void cpp_main(void) 
{
    NVIC_DisableIRQ(EXTI1_IRQn);
    NVIC_DisableIRQ(EXTI2_IRQn);

    HAL_TIM_Base_Start_IT(&htim4);

    res_icm42688 = icm42688.begin();

    HAL_Delay(10);

    res_general = icm42688.general.enable_acc(ICM42688::General::ACCEL_ODR::ACCEL_ODR_200HZ, 
                               ICM42688::General::ACCEL_FS::ACCEL_FS_2G, 
                               ICM42688::General::FILTER_LEVEL::MEDIUM);

    res_general = icm42688.general.enable_gyro(ICM42688::General::GYRO_ODR::GYRO_ODR_200HZ,
                                ICM42688::General::GYRO_FS::GYRO_FS_500DPS,
                                ICM42688::General::FILTER_LEVEL::MEDIUM);

    res_general = icm42688.general.enable_temp();

    res_general = icm42688.general.enable_tmst(ICM42688::General::TMST_RESOL::TMST_RESOL_1us);

    HAL_Delay(200);   // 必要延时,等待传感器稳定

    res_general = icm42688.general.set_acc_gyro_offset(500);

    res_Int = icm42688.Int.set_int1_pin_cfg(ICM_INT1_GPIO_Port, ICM_INT1_Pin,
                                                ICM42688::INT::LEVEL::LEVEL_ACTIVE_HIGH,
                                                ICM42688::INT::DRIVE::DRIVE_PUSH_PULL,
                                                ICM42688::INT::MODE::LATCHED);
    NVIC_EnableIRQ(EXTI1_IRQn);

    res_Int = icm42688.Int.set_int2_pin_cfg(ICM_INT2_GPIO_Port, ICM_INT2_Pin,
                                                ICM42688::INT::LEVEL::LEVEL_ACTIVE_HIGH,
                                                ICM42688::INT::DRIVE::DRIVE_PUSH_PULL,
                                                ICM42688::INT::MODE::LATCHED);
    NVIC_EnableIRQ(EXTI2_IRQn);

    res_Int = icm42688.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                                                ICM42688::INT::INT_NUM::INT1,
                                                test,
                                                &icm42688,
                                                ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGH);
    #ifdef ENABLE_DEBUG    
        icm42688.Int.print_int_source();
    #endif

    HAL_Delay(1000);

    while(1)
    {
        // icm42688.general.read_data();
        // euler.MadgwickAHRSupdateIMU(icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az(),
        //                          icm42688.general.get_gx(), icm42688.general.get_gy(), icm42688.general.get_gz());

        // printf("%f,%f,%f,%f,%f,%f\n", 
        // icm42688.general.get_ax(), icm42688.general.get_ay(), icm42688.general.get_az(),
        // icm42688.general.get_gx(), icm42688.general.get_gy(), icm42688.general.get_gz());

        // printf("%.3f,%.3f,%.3f\n", euler.get_pitch(), euler.get_roll(), euler.get_yaw());



        // HAL_Delay(1);
    }
}
