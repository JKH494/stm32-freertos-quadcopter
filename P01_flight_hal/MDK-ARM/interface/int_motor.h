#ifndef INT_MOTOR_H
#define INT_MOTOR_H

#include "tim.h"
#include "Com_debug.h"

typedef struct {
    TIM_HandleTypeDef * tim;
    uint16_t channel;
    int16_t speed;
} motor_struct;

/**
 * @brief 传入参数实际上是比较值CCR,默认值200，最大为1000
 * 
 * @param speed 
 */
void int_motor_set_speed(motor_struct *motor);

/**
 * @brief 启动电机
 * 
 * @param motor 
 */
void int_motor_start(motor_struct *motor);

#endif // INT_MOTOR_H
