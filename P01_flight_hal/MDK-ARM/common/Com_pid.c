#include "Com_pid.h"

//通用单次PID计算
void Com_PID_Calc(PID_Struct *pid)
{
    //计算误差
    pid->error = pid->target - pid->measure;

    //计算积分
    pid->integral += pid->error;

    //计算微分
      //初次计算，上次误差等于当前误差
    if (pid->last_error == 0) {
        pid->last_error = pid->error;
    }

    float der = pid->error - pid->last_error;

    //计算输出
    pid->output = pid->Kp * pid->error + (pid->Ki * pid->integral * PID_PERIOD ) 
        + (pid->Kd * der / PID_PERIOD);

    //更新上次误差
    pid->last_error = pid->error;
}

//BMP280的PID计算
void Com_PID_Calc_BMP280(PID_Struct *pid)
{
    //计算误差
    pid->error = pid->target - pid->measure;

    //计算积分
    pid->integral += pid->error;

    //计算微分
      //初次计算，上次误差等于当前误差
    if (pid->last_error == 0) {
        pid->last_error = pid->error;
    }

    float der = pid->error - pid->last_error;

    //计算输出
    pid->output = pid->Kp * pid->error + (pid->Ki * pid->integral * PID_BMP280_PERIOD ) 
        + (pid->Kd * der / PID_BMP280_PERIOD);

    //更新上次误差
    pid->last_error = pid->error;
}

//串级PID计算
void Com_PID_Calc_Chain(PID_Struct *pid_outer, PID_Struct *pid_inner)
{
    //先计算外环
    Com_PID_Calc(pid_outer);
    //将外环输出当作内环目标值
    pid_inner->target = pid_outer->output ;
    //计算内环
    Com_PID_Calc(pid_inner);
}


/**
 * @brief 限幅函数
 * 
 * @param speed 
 * @param max_speed 
 * @param min_speed 
 * @return int16_t 
 */
int16_t Com_limit_output(int16_t speed,int16_t max_speed,int16_t min_speed)
{
    if (speed > max_speed) {
        return max_speed;
    } 
    else if (speed < min_speed) 
    {
        return min_speed;
    }
    return speed;
}
