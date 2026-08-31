#ifndef APP_PROCESS_DATA_H
#define APP_PROCESS_DATA_H

#include "int_joystick.h"
#include "int_key.h"
#include "Lcd_Driver.h"
#include "Com_tool.h"  
#include "App_transmit_data.h" 

//OLED结构体
typedef struct
{
    uint8_t bmp280_init_status;
    int16_t bmp280_altitude_dm;
    Key_type last_key;
    TickType_t key_tick;
} Oled_Data;
extern Oled_Data oled_data;

typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t rol;
    int16_t pit;
    uint8_t shutdown;       //1：切换开关机 0：不切换
    uint8_t fix_height;     //1：切换高度锁定 0：不切换
}Remote_data;

/**
 * @brief 处理按键数据，如果有按键按下，进行记录
 * 
 */
void App_process_key_data(void);

/** 
 * @brief 处理摇杆数据，修正极性相位和标准值
 * 
 */
void App_process_joystick_data(void);

#endif // APP_PROCESS_DATA_H
