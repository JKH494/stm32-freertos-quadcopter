#include "int_joystick.h"

uint16_t adc_data[4];

/**
 * @brief 初始化摇杆，开启ADC采集
 * 
 */
void int_joystick_init(void)
{
    //使用HAL库初始化使用DMA的ADC
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data, 4);   
}


/**
 * @brief 读取摇杆数据,保存到结构体
 * 
 * @param js 指向joystick_struct结构体的指针，用于存储摇杆数据
 */
void int_joystick_get(joystick_struct *joystick)
{
    joystick->thr = adc_data[0];
    joystick->yaw = adc_data[3];
    joystick->rol = adc_data[2];
    joystick->pit = adc_data[1];
}
