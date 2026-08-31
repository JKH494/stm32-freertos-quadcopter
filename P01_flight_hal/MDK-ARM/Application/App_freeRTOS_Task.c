#include "App_freeRTOS_Task.h"
//STM32F103C8T6 => SRAM 20k  => 分配12K给操作系统


//LED结构体
led_struct left_top_led = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
led_struct right_top_led = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
led_struct right_bottom_led = {.port = LED3_GPIO_Port, .pin = LED3_Pin};
led_struct left_bottom_led = {.port = LED4_GPIO_Port, .pin = LED4_Pin};

//表示当前连接状态
Remote_State remote_state = REMOTE_DISCONNECTED;

//表示当前飞行模式
Flight_State flight_state = IDLE;
extern int16_t bmp280_telem_alt_dm;

//遥感数据
Remote_data remote_data = {.rol = 500, .pit = 500, .yaw = 500, .thr = 0, .fix_height = 0, .shutdown = 0};

//全局变量存储电量数据

//电源管理任务
void power_task(void *args);
// 最小推荐填写128 => 128*4 = 512B
#define POWER_TASK_STACK_SIZE 128
// 任务优先级 => 数值越小 优先级越小  => 最大4  => 不推荐使用最小优先级0
#define POWER_TASK_PRIORITY 4
TaskHandle_t power_task_handle;
//定义任务周期
#define POWER_TASK_PERIOD 10000

//飞行控制任务
void flight_task(void *args);
#define FLIGHT_TASK_STACK_SIZE 192
#define FLIGHT_TASK_PRIORITY 3
TaskHandle_t flight_task_handle;
#define FLIGHT_TASK_PERIOD 6


//2.4G通信任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 2
TaskHandle_t com_task_handle;
#define COM_TASK_PERIOD 6

//LED任务
void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_task_handle;
#define LED_TASK_PERIOD 100


/**
 * @brief 启动freeRTOS操作系统
 *
 */
void App_freeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    //2. 创建飞行控制任务
    xTaskCreate(flight_task, "flight_task", FLIGHT_TASK_STACK_SIZE, NULL, FLIGHT_TASK_PRIORITY, &flight_task_handle);
    
    //3. 创建LED任务
    xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_task_handle);

    //4. 创建2.4G通信任务
    xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 5. 启动调度器
    vTaskStartScheduler();
}


void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        // //每十秒执行一次，启动电源防止关机
        // vTaskDelayUntil(&xLastWakeTime, POWER_TASK_PERIOD);
        // //启动电源
        // Int_IP5305T_start();

        //使用任务通知的方式,收到任务通知 ulNotificationValue = 1，否则 ulNotificationValue = 0
        uint32_t ulNotificationValue = ulTaskNotifyTake( pdTRUE, POWER_TASK_PERIOD );
        if(ulNotificationValue != 0)
        {
            Int_IP5305T_shutdown();
        }
        else
        {
            Int_IP5305T_start();
        }
    }
}

void flight_task(void *args)
{
    //初始化电机、MPU6050和BMP280
    App_flight_init();

    //所有初始化完成后再设置任务周期基准
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    while(1)
    {
        //每6ms执行一次，进行飞行控制
        App_flight_get_euler_angle();
        App_flight_euler_pid();

        if(flight_state == FIX_HEIGHT)
        {
            count ++;
            if(count >= 10)
            {
                App_flight_bmp280_pid();
                count = 0;
            }
        }
        else
        {
            count = 0;
        }

        App_flight_control_motor();

        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_PERIOD);
    }
}

void led_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count = 0;
    while(1)
    {
        count ++;

        //前两个灯表示当前连接状态
        //判断当前连接状态
        if(remote_state == REMOTE_CONNECTED)
        {
            //点亮前两个灯
            int_led_turn_on(&left_top_led);
            int_led_turn_on(&right_top_led);
        }
        else
        {
            //熄灭前两个灯
            int_led_turn_off(&left_top_led);
            int_led_turn_off(&right_top_led);
        }

        //后两个灯表示当前飞行模式
        //判断当前飞行模式
        if(flight_state == IDLE)
        {
            //后两个灯慢速闪烁，500ms闪烁一次
            if(count % 5 == 0)
            {
                int_led_toggle(&left_bottom_led);
                int_led_toggle(&right_bottom_led);
            }
        }
        else if(flight_state == NORMAL)
        {
            //后两个灯快速闪烁，200ms闪烁一次
            if(count % 2 == 0)
            {
                int_led_toggle(&left_bottom_led);
                int_led_toggle(&right_bottom_led);
            }
        }
        else if(flight_state == FIX_HEIGHT)
        {
            //后两个灯常量
            int_led_turn_on(&left_bottom_led);
            int_led_turn_on(&right_bottom_led);
        }
        else if(flight_state == FAIL)
        {
            //后两个灯熄灭
            int_led_turn_off(&left_bottom_led);
            int_led_turn_off(&right_bottom_led);
        }

        if(count == 10)
        {
            count = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, LED_TASK_PERIOD);
    }
}

void com_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t last_battery_print = xLastWakeTime;
    //开启电量检测ADC
    while(1)
    {
        //处理连接状态
        uint8_t ret = App_receive_data();       
        App_process_connection_state(ret);
        //处理关机指令
        if(remote_data.shutdown == 1)
        {
            // Int_IP5305T_shutdown();
            //使用FReeRTOS的任务通知
            xTaskNotifyGive(power_task_handle);
        }
        //处理飞行状态
        App_process_flight_state();

        //准备电量回传
        TickType_t now = xTaskGetTickCount();
        float pre_t, temp_t, alt_t;
        if((now - last_battery_print) >= pdMS_TO_TICKS(500))
        {
            if(BMP280GetData(&pre_t, &temp_t, &alt_t))
            {
                bmp280_telem_alt_dm = (int16_t)(alt_t * 10.0f);
            }
            last_battery_print = now;
        }
        //每6ms执行一次，进行2.4G通信
        vTaskDelay(COM_TASK_PERIOD);
    }

}
