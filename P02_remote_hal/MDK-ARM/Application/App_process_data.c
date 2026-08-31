#include "FreeRTOS.h"
#include "semphr.h"
#include "App_process_data.h"


//摇杆结构体
joystick_struct joystick = {0};
Remote_data remote_data = {0};
//区分摇杆控制值和按键微调值
int16_t key_pit_offset = 0;
int16_t key_rol_offset = 0;
//电池电压值
extern volatile uint8_t bmp280_init_status;
extern volatile int16_t bmp280_altitude_dm;

//摇杆偏移量
int16_t thr_offset = 0;
int16_t yaw_offset = 0;
int16_t rol_offset = 0;
int16_t pit_offset = 0;

/**
 * @brief 零偏校准函数
 * 
 */
void App_calibrate_offset(void)
{
    //临界区清除按键偏移值
    taskENTER_CRITICAL();
    key_pit_offset = 0;
    key_rol_offset = 0;
    taskEXIT_CRITICAL();

    int16_t yaw_sum = 0;
    int16_t rol_sum = 0;
    int16_t pit_sum = 0;
    int16_t thr_sum = 0;

    //多次读取求平均值
    for(uint8_t i = 0; i < 10; i++)
    {
        App_process_joystick_data();
        yaw_sum += joystick.yaw - 500;
        rol_sum += joystick.rol - 500;
        pit_sum += joystick.pit - 500;
        thr_sum += joystick.thr - 0;
        vTaskDelay(10);
    }
    
    //临界区保护
    taskENTER_CRITICAL();
    yaw_offset += yaw_sum / 10;
    rol_offset += rol_sum / 10;
    pit_offset += pit_sum / 10;
    thr_offset += thr_sum / 10;
    taskEXIT_CRITICAL();
}


/**
 * @brief 处理按键数据，如果有按键按下，进行记录
 * 
 */
void App_process_key_data(void)
{
    Key_type key = Key_Get();

    if (key != KEY_NONE)
    {
        taskENTER_CRITICAL();
        oled_data.last_key = key;
        oled_data.key_tick = xTaskGetTickCount();
        taskEXIT_CRITICAL();
    }
    //根据按键值，修改对应的俯仰、横滚的值，在摇杆校准时清除按键偏移值
    if (key == KEY_UP)
    {
        taskENTER_CRITICAL();
        key_pit_offset += 10;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_DOWN)
    {
        taskENTER_CRITICAL();
        key_pit_offset -= 10;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_LEFT)
    {
        taskENTER_CRITICAL();
        key_rol_offset -= 10;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_RIGHT)
    {
        taskENTER_CRITICAL();
        key_rol_offset += 10;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_LEFT_X)
    {
        taskENTER_CRITICAL();
        remote_data.shutdown = 1 ;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_RIGHT_X)
    {
        taskENTER_CRITICAL();
        remote_data.fix_height = 1;
        taskEXIT_CRITICAL();
    }
    else if (key == KEY_RIGHT_X_LONG)
    {
        //触发校准
        App_calibrate_offset();
    }  
}

/** 
 * @brief 处理摇杆数据，修正极性相位和标准值
 * 
 */
void App_process_joystick_data(void)
{
    int_joystick_get(&joystick);

    //临界区
    taskENTER_CRITICAL();
    
    //处理范围和极性，想要的范围是 1000，ADC是4095

    joystick.thr = joystick.thr * 1000 / 4095 ;
    joystick.yaw = joystick.yaw * 1000 / 4095 ;
    joystick.rol = joystick.rol * 1000 / 4095 ;
    joystick.pit = joystick.pit * 1000 / 4095 ;

    //处理零偏
    joystick.thr -= thr_offset;
    joystick.yaw -= yaw_offset - 45;
    joystick.rol -= rol_offset;
    joystick.pit -= pit_offset;

    //加上按键偏移值
    joystick.pit += key_pit_offset;
    joystick.rol += key_rol_offset;

    //限制在0 - 1000
    joystick.thr = Com_limit(joystick.thr, 0, 1000);
    joystick.yaw = Com_limit(joystick.yaw, 0, 1000);
    joystick.rol = Com_limit(joystick.rol, 0, 1000);
    joystick.pit = Com_limit(joystick.pit, 0, 1000);

    //赋值
    remote_data.thr = joystick.thr;
    remote_data.yaw = joystick.yaw;
    remote_data.rol = joystick.rol;
    remote_data.pit = joystick.pit;

    //退出临界区
    taskEXIT_CRITICAL();
}
