#ifndef APP_TRANSMIT_DATA_H
#define APP_TRANSMIT_DATA_H

#include "int_SI24R1.h"
#include "App_process_data.h"
#include "FreeRTOS.h"
#include "Task.h"

//定义帧头 3字节
#define FRAME_HEADER_1  'j'
#define FRAME_HEADER_2  'k'
#define FRAME_HEADER_3  'h'


/**
 * @brief 自动切换模式，并发送数据
 * 
 */
void App_transmit_data(void);


#endif // APP_TRANSMIT_DATA_H
