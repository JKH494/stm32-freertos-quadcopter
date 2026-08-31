#include "App_receive_data.h"

extern Remote_data remote_data ;

uint8_t rx_data[TX_PLOAD_WIDTH] = {0};

//最后一次收到有效数据包的时间
static TickType_t last_valid_rx_tick = 0;

//表示当前连接状态
extern Remote_State remote_state ;

//飞行状态
extern  Flight_State flight_state;

//油门状态
Thr_State thr_state = FREE;

//MAX时间
uint32_t max_time = 0;
//MIN时间
uint32_t min_time = 0;

//电压值
extern bool bmp280_ready;
static uint8_t tx_data[TX_PLOAD_WIDTH] = {0};

//定高pid
extern PID_Struct pid_height;

//记录定高目标值和油门
float pre;
float temp;
float BMP_asl;
uint16_t BMP_thr;
int16_t bmp280_telem_alt_dm = 0;

//回传组帧函数
static void App_send_telemetry_data(void)
{
    uint32_t check_sum = 0;


    memset(tx_data, 0, sizeof(tx_data));

    tx_data[0] = FRAME_HEADER_1;
    tx_data[1] = FRAME_HEADER_2;
    tx_data[2] = FRAME_HEADER_3;

    tx_data[3] = (uint8_t)(bmp280_ready ? 0 : 1);
    tx_data[4] = (uint8_t)(bmp280_telem_alt_dm >> 8);
    tx_data[5] = (uint8_t)bmp280_telem_alt_dm;

    for (uint8_t i = 0; i < 13; i++)
    {
        check_sum += tx_data[i];
    }

    tx_data[13] = (uint8_t)(check_sum >> 24);
    tx_data[14] = (uint8_t)(check_sum >> 16);
    tx_data[15] = (uint8_t)(check_sum >> 8);
    tx_data[16] = (uint8_t)check_sum;

    Int_SI24R1_TX_Mode();
    Int_SI24R1_TxPacket(tx_data);
    Int_SI24R1_RX_Mode();
}


/**
 * @brief 接收遥感数据，并解析为结构体
 * 
 * @return uint8_t 0：接收成功   1：接收失败
 */
uint8_t App_receive_data(void)
{
    memset(rx_data, 0, TX_PLOAD_WIDTH);
    uint8_t ret = Int_SI24R1_RxPacket(rx_data);

    if (ret != 0) 
    {
        return 1; // 接收失败
    }
    
    // 解析接收到的数据
    //帧头校验
    if(rx_data[0] != FRAME_HEADER_1 || rx_data[1] != FRAME_HEADER_2 || rx_data[2] != FRAME_HEADER_3)
    {
        return 1;
    }
    //帧尾校验
    uint32_t sum = 0;
    uint32_t sum_receive = 0;
    for(uint8_t i = 0; i < 13; i++)
    {
        sum += rx_data[i];
    }
     //高位在前
    sum_receive = rx_data[13] << 24 | rx_data[14] << 16 | rx_data[15] << 8 | rx_data[16];
    if(sum != sum_receive)
    {
        return 1;
    }
    //发送电压数据
    App_send_telemetry_data();

    //保存数据
    remote_data.thr = (rx_data[3] << 8) | rx_data[4];
    remote_data.yaw = (rx_data[5] << 8) | rx_data[6];
    remote_data.pit = (rx_data[7] << 8) | rx_data[8];
    remote_data.rol = (rx_data[9] << 8) | rx_data[10];
    remote_data.shutdown = rx_data[11];
    remote_data.fix_height = rx_data[12];

    // debug_printf("thr: %d, yaw: %d, pit: %d, rol: %d, shutdown: %d, fix_height: %d\r\n",
    //     remote_data.thr, remote_data.yaw, remote_data.pit, remote_data.rol, remote_data.shutdown, remote_data.fix_height);

    return 0;
}


/**
 * @brief 处理连接状态
 * 
 * @param 根据上次接受的返回值，处理连接状态 
 */
void App_process_connection_state(uint8_t res)
{
    TickType_t now = xTaskGetTickCount();

    if(res == 0)
    {
        last_valid_rx_tick = now;
        remote_state = REMOTE_CONNECTED;
    }
    else if((remote_state == REMOTE_CONNECTED) &&
            ((now - last_valid_rx_tick) >= pdMS_TO_TICKS(REMOTE_TIMEOUT_MS)))
    {
        remote_state = REMOTE_DISCONNECTED;
    }
}

/**
 * @brief  判断解锁           
 * 
 * @return uint8_t 0：解锁 1：不解锁
 */
uint8_t App_process_unlock(void)
{
    TickType_t now = xTaskGetTickCount();

    // 失联时禁止使用旧遥控数据继续解锁
    if(remote_state != REMOTE_CONNECTED)
    {
        thr_state = FREE;
        return 1;
    }

    switch (thr_state)
    {
        case FREE:
            if(remote_data.thr > 900)
            {
                thr_state = MAX;
                max_time = now;
            }
            break;
        case MAX:
            if(remote_data.thr <= 900)
            {
                thr_state = FREE;
            }
            else if((now - max_time) >= pdMS_TO_TICKS(1000))
            {
                thr_state = LEAVE_MAX;
            }
            break;
        case LEAVE_MAX:
            if(remote_data.thr <= 100)
            {
                thr_state = MIN;
                min_time = now;
            }
            break;
        case MIN:
            if(remote_data.thr > 100)
            {
                thr_state = FREE;
            }
            else if((now - min_time) >= pdMS_TO_TICKS(1000))
            {
                thr_state = UNLOCK;
            }
            break;
        case UNLOCK:
            return 0;
        default:
            thr_state = FREE;
            break;
    }

    return 1;
}

/**
 * @brief 处理飞行状态
 * 
 */
void App_process_flight_state(void)
{
    static uint8_t prev_fix_height = 0;
    //使用状态机实现
    switch (flight_state)
    {
        case IDLE:
            //判断是否解锁
            if (App_process_unlock() == 0)
            {
                flight_state = NORMAL ;
                thr_state = FREE;
            }            
            break;
        case NORMAL:
            //进入定高状态

            if(remote_data.fix_height == 1 && prev_fix_height == 0)
            {
                // 此时仍然是NORMAL，飞行任务不会读取BMP280
                if (BMP280GetData(&pre, &temp, &BMP_asl))
                {
                    pid_height.error = 0.0f;
                    pid_height.last_error = 0.0f;
                    pid_height.integral = 0.0f;
                    pid_height.output = 0.0f;

                    BMP_thr = remote_data.thr;

                    flight_state = FIX_HEIGHT;  // 最后发布状态
                }
            }
            //进入故障状态
            if(remote_state == REMOTE_DISCONNECTED)
            {
                flight_state = FAIL;
            }
            break;
        case FIX_HEIGHT:
            //退出定高状态
            if(remote_data.fix_height == 1 && prev_fix_height == 0)
            {
                flight_state = NORMAL;
            }
            //进入故障状态
            if(remote_state == REMOTE_DISCONNECTED)
            {
                flight_state = FAIL;
            }
            break;
        case FAIL:
            //处理故障，进入待机模式
            vTaskDelay(1);
            flight_state = IDLE;
            break;
        default:
            break;
    }
    prev_fix_height = remote_data.fix_height;
}  
