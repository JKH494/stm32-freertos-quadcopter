#ifndef APP_FLIGHT_H
#define APP_FLIGHT_H

#include "int_MPU6050.h"
#include "int_motor.h"
#include "Com_debug.h"
#include "Com_filter.h"
// #include "math.h"
#include "Com_IMU.h"
#include "Com_pid.h"
#include "int_bmp280.h"

/**
 * @brief 根据陀螺仪数据计算欧拉角
 */
void App_flight_get_euler_angle(void);

/**
 * @brief 根据欧拉角pid控制
 * 
 */
void App_flight_euler_pid(void);


/**
 * @brief 根据bmp280数据处理
 * 
 */
void App_flight_bmp280_pid(void);


/**
 * @brief 根据PID控制量控制电机
 * 
 */
void App_flight_control_motor(void);

/**
 * @brief 初始化MPU6050和电机
 * 
 */
void App_flight_init(void);

#endif // APP_FLIGHT_H
