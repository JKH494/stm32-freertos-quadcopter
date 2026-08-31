#ifndef COM_PID_H
#define COM_PID_H

#include "main.h"

#define PID_PERIOD 0.006
#define PID_BMP280_PERIOD 0.060

// PID结构体
typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float target;     //目标值
    float error;       //误差值
    float last_error;  //上一次误差值
    float integral;    //积分值
    float measure;     //测量值
    float output;      //输出值
} PID_Struct;

//单次PID计算
void Com_PID_Calc(PID_Struct *pid);
//BMP280的PID计算
void Com_PID_Calc_BMP280(PID_Struct *pid);

//串级PID计算
void Com_PID_Calc_Chain(PID_Struct *pid_outer, PID_Struct *pid_inner);

/**
 * @brief 限幅函数
 * 
 * @param speed 
 * @param max_speed 
 * @param min_speed 
 * @return int16_t 
 */
int16_t Com_limit_output(int16_t speed,int16_t max_speed,int16_t min_speed);

#endif // COM_PID_H
