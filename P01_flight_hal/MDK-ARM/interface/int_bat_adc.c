#include "int_bat_adc.h"
  
/**
 * @brief 初始化ADC
 * 
 */
void int_bat_adc_init(void)
{
    // 初始化ADC模块
    //打开使能
    HAL_GPIO_WritePin(BAT_ADC_EN_GPIO_Port,BAT_ADC_EN_Pin,GPIO_PIN_SET);
    //打开ADC
    HAL_ADC_Start(&hadc1);
}

/**
 * @brief 读取电压值
 * 
 * @return float  返回电压值
 */
float int_bat_adc_read(void)
{
    //读取ADC值
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
    
    //转换为电压值
    float voltage = (adc_value * 3.3f) / 4095.0f * 2.0f;
    
    return voltage;
}
