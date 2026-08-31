#ifndef __APP_FREERTOS_TASK__
#define __APP_FREERTOS_TASK__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "int_IP5305T.h"
#include "int_SI24R1.h"
#include "App_process_data.h"
#include "Lcd_Driver.h"
#include "App_transmit_data.h"


/**
 * @brief 启动freeRTOS操作系统
 * 
 */
void App_freeRTOS_start(void);

#endif // __APP_FREERTOS_TASK__
