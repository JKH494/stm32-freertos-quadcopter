#ifndef INT_BAT_ADC_H
#define INT_BAT_ADC_H

#include "adc.h"

/**
 * @brief 初始化ADC
 * 
 */
void int_bat_adc_init(void);

/**
 * @brief 读取电压值
 * 
 * @return float  返回电压值
 */
float int_bat_adc_read(void);


#endif // INT_BAT_ADC_H
