#include "int_motor.h"

/**
 * @brief 传入参数实际上是比较值CCR,默认值200，最大为1000
 * 
 * @param speed 
 */
void int_motor_set_speed(motor_struct *motor)
{
    if(motor->speed > 1000)
    {
        debug_printf("speed is too large\n");
        return;
    }

    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);    
}

void int_motor_start(motor_struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);
    HAL_TIM_PWM_Start(motor->tim, motor->channel);
}
