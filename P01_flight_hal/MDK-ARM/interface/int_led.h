#ifndef INT_LED_H
#define INT_LED_H

#include "main.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} led_struct;

/**
 * @brief 打开LED灯
 * 
 * @param led 
 */
void int_led_turn_on(led_struct *led);
/**
 * @brief 关闭LED灯
 * 
 * @param led 
 */
void int_led_turn_off(led_struct *led);
/**
 * @brief 翻转LED灯
 * 
 * @param led 
 */
void int_led_toggle(led_struct *led);

#endif // INT_LED_H
