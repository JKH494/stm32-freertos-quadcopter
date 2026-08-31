#include "int_led.h"

/**
 * @brief 打开LED灯
 * 
 * @param led 
 */
void int_led_turn_on(led_struct *led)
{
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭LED灯
 * 
 * @param led 
 */
void int_led_turn_off(led_struct *led)
{
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
}

/**
 * @brief 翻转LED灯
 * 
 * @param led 
 */
void int_led_toggle(led_struct *led)
{
    HAL_GPIO_TogglePin(led->port, led->pin);
}
