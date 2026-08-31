#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "int_IP5305T.h"
#include "int_motor.h"
#include "int_led.h"
#include "Com_config.h"
#include "int_SI24R1.h"
#include "App_receive_data.h"
#include "App_flight.h"
#include "adc.h"
#include "int_bat_adc.h"
#include "int_bmp280.h"

/**
 * @brief 启动freeRTOS操作系统
 * 
 */
void App_freeRTOS_start(void);

#endif // __APP_FREERTOS_TASK_

