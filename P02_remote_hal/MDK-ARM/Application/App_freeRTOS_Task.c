#include "App_freeRTOS_Task.h"

//通讯缓冲区
uint8_t com_tx_buf[TX_PLOAD_WIDTH] = {0};

Oled_Data oled_data = {0};
//遥控全部数据结构体
extern Remote_data remote_data;
//接收的飞机电压
extern volatile uint8_t bmp280_init_status;
extern volatile int16_t bmp280_altitude_dm;

//Set to 0 after the BMP280 test to restore the normal remote page.
//摇杆数据结构体
extern joystick_struct joystick;

//STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统
//电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;
//电源管理周期
#define POWER_TASK_PERIOD 10000

//2.4G通信任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t com_task_handle;
//通讯周期
#define COM_TASK_PERIOD 6

//按键任务
void key_task(void *args);
#define KEY_TASK_STACK_SIZE 128
#define KEY_TASK_PRIORITY 2
TaskHandle_t key_task_handle;
//按键周期
#define KEY_TASK_PERIOD 30

//摇杆任务
void joystick_task(void *args);
#define JOYSTICK_TASK_STACK_SIZE 128
#define JOYSTICK_TASK_PRIORITY 2
TaskHandle_t joystick_task_handle;
//摇杆周期
#define JOYSTICK_TASK_PERIOD 100

//OLED任务
void oled_task(void *args);
#define OLED_TASK_STACK_SIZE 128
#define OLED_TASK_PRIORITY 2
TaskHandle_t oled_task_handle;
//OLED周期
#define OLED_TASK_PERIOD 100

/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);
    // 2. 创建2.4G通信任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);
    // 3. 创建按键任务
    xTaskCreate(key_task, "key_task", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &key_task_handle);
    // 4. 创建摇杆任务
    xTaskCreate(joystick_task, "joystick_task", JOYSTICK_TASK_STACK_SIZE, NULL, JOYSTICK_TASK_PRIORITY, &joystick_task_handle);
    // 5. 创建OLED任务
    xTaskCreate(oled_task, "oled_task", OLED_TASK_STACK_SIZE, NULL, OLED_TASK_PRIORITY, &oled_task_handle);
    // 6. 启动调度器
    vTaskStartScheduler();
}


void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        //每十秒执行一次，启动电源防止关机
        vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);
        //启动电源
        Int_IP5305T_start();
    }
}

void key_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        App_process_key_data();

        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_PERIOD);

    }
}

void com_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        //将打包完成的数据发送
        App_transmit_data();

        //每6ms执行一次，进行2.4G通信
        vTaskDelay( COM_TASK_PERIOD);
    }
}

void joystick_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    int_joystick_init();

    while (1)
    {
        App_process_joystick_data();

        vTaskDelayUntil(&xLastWakeTime, JOYSTICK_TASK_PERIOD);
    }
}

//供OLED任务使用的转换函数
static const char *App_key_to_string(Key_type key)
{
    switch (key)
    {
        case KEY_UP:
            return "KEY_UP";

        case KEY_DOWN:
            return "KEY_DOWN";

        case KEY_LEFT:
            return "KEY_LEFT";

        case KEY_RIGHT:
            return "KEY_RIGHT";

        case KEY_LEFT_X:
            return "KEY_LEFT_X";

        case KEY_RIGHT_X:
            return "KEY_RIGHT_X";

        case KEY_RIGHT_X_LONG:
            return "KEY_RIGHT_X_LONG";

        case KEY_NONE:
            return "KEY_NONE";

        default:
            return "KEY_UNKNOWN";
    }
}
void oled_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        OLED_Clear();

        OLED_ShowString(0, 16, (u8 *)"                ", 16, 1);
        if (bmp280_init_status == 0)
        {
            OLED_ShowString(0, 16, (u8 *)"BMP:OK ", 16, 1);
            OLED_ShowNum(56, 16, (u32)(bmp280_altitude_dm >= 0 ? bmp280_altitude_dm : 0), 4, 16, 1);
            OLED_ShowString(88, 16, (u8 *)"dm", 16, 1);
        }
        else
        {
            OLED_ShowString(0, 16, (u8 *)"BMP:FAIL", 16, 1);
        }

        OLED_ShowString(0, 0,(u8 *)App_key_to_string(oled_data.last_key),16, 1);
                        
        OLED_ShowString(0, 32, (u8 *)"T:", 16, 1);
        OLED_ShowNum(16, 32, joystick.thr, 4, 16, 1);
        OLED_ShowString(64, 32, (u8 *)"Y:", 16, 1);
        OLED_ShowNum(80, 32, joystick.yaw, 4, 16, 1);

        OLED_ShowString(0, 48, (u8 *)"R:", 16, 1);
        OLED_ShowNum(16, 48, joystick.rol, 4, 16, 1);
        OLED_ShowString(64, 48, (u8 *)"P:", 16, 1);
        OLED_ShowNum(80, 48, joystick.pit, 4, 16, 1);

        OLED_Refresh();
        vTaskDelayUntil(&xLastWakeTime, OLED_TASK_PERIOD);
    }
}
