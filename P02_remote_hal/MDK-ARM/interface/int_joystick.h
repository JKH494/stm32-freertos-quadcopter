#ifndef INT_JOYSTICK_H
#define INT_JOYSTICK_H

#include "adc.h"

typedef struct {
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;  
} joystick_struct;

/**
 * @brief 初始化摇杆，开启ADC采集
 * 
 */
void int_joystick_init(void);
/**
 * @brief 读取摇杆数据,保存到结构体
 * 
 * @param js 指向joystick_struct结构体的指针，用于存储摇杆数据
 */
void int_joystick_get(joystick_struct *joystick);

#endif // INT_JOYSTICK_H
