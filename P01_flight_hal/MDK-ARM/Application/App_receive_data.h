#ifndef APP_RECEIVE_DATA_H
#define APP_RECEIVE_DATA_H

#include "int_SI24R1.h"
#include "Com_config.h"
#include "int_bmp280.h"
#include "Com_pid.h"

//定义帧头 3字节
#define FRAME_HEADER_1  'j'
#define FRAME_HEADER_2  'k'
#define FRAME_HEADER_3  'h'

//超过该时间没有收到有效数据包，判定遥控器断开
#define REMOTE_TIMEOUT_MS 200U

/**
 * @brief 接收遥感数据，并解析为结构体
 * 
 * @return uint8_t 0：接收成功   1：接收失败
 */
uint8_t App_receive_data(void);

/**
 * @brief 处理连接状态
 * 
 * @param 根据上次接受的返回值，处理连接状态 
 */
void App_process_connection_state(uint8_t res);

/**
 * @brief 处理飞行状态
 * 
 */
void App_process_flight_state(void);

#endif // APP_RECEIVE_DATA_H
