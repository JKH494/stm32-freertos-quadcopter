#ifndef COM_CONFIG_H
#define COM_CONFIG_H

#include "main.h"

// 表示当前连接状态
typedef enum{
    REMOTE_CONNECTED,
    REMOTE_DISCONNECTED,
}Remote_State;

//飞行状态
typedef enum{
    IDLE = 0,
    NORMAL,
    FIX_HEIGHT,
    FAIL,
}Flight_State;

//油门状态
typedef enum{
    FREE = 0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
}Thr_State;

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;
    uint8_t shutdown;       //1：切换开关机 0：不切换
    uint8_t fix_height;     //1：切换高度锁定 0：不切换
}Remote_data;

typedef enum
{
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEFT_X,
    KEY_RIGHT_X,
    KEY_RIGHT_X_LONG
} Remote_key;

//角速度结构体
typedef struct
{
    int16_t Gyro_X; //向右飞为正 ，横滚角
    int16_t Gyro_Y; //向前飞为正 ，俯仰角
    int16_t Gyro_Z; //顺时针为负 ，偏航角
}Gyro_Struct;
//加速度结构体
typedef struct
{
    int16_t Accel_X; //向前为正
    int16_t Accel_Y; //向左为正
    int16_t Accel_Z; //向上为正
}Accel_Struct;
//角速度和加速度结构体
typedef struct
{
    Gyro_Struct gyro;
    Accel_Struct accel;
}Gyro_Accel_Struct;
//解算得到的欧拉角
typedef struct
{
    float Yaw; 
    float Pitch; 
    float Roll;
}Euler_Struct;

#endif // COM_CONFIG_H

