#include "App_transmit_data.h"

extern Remote_data remote_data;

//发送数据缓冲区
uint8_t tx_buf[TX_PLOAD_WIDTH] = {0};
uint8_t rx_buf[TX_PLOAD_WIDTH] = {0};

//接受电压值
volatile uint8_t bmp280_init_status;
volatile int16_t bmp280_altitude_dm;

static uint8_t App_validate_telemetry(const uint8_t *data)
{
    uint32_t check_sum = 0;
    uint32_t received_sum;

    if((data[0] != FRAME_HEADER_1) ||
       (data[1] != FRAME_HEADER_2) ||
       (data[2] != FRAME_HEADER_3))
    {
        return 1;
    }

    for(uint8_t i = 0; i < 13; i++)
    {
        check_sum += data[i];
    }

    received_sum = ((uint32_t)data[13] << 24) |
                   ((uint32_t)data[14] << 16) |
                   ((uint32_t)data[15] << 8) |
                   (uint32_t)data[16];

    return (check_sum == received_sum) ? 0 : 1;
}

/**
 * @brief 自动切换模式，并发送数据
 * 
 */
void App_transmit_data(void)
{
    //切换到发送模式
    Int_SI24R1_TX_Mode();

    //32位校验和
    uint32_t check_sum = 0;

    //发送数据
    //帧头校验3字节  数据本身10字节  帧尾校验4字节
    tx_buf[0] = FRAME_HEADER_1;
    tx_buf[1] = FRAME_HEADER_2;
    tx_buf[2] = FRAME_HEADER_3;
    
    //数据高位先行
    tx_buf[3] = ( remote_data.thr >> 8 ) & 0xFF;
    tx_buf[4] = remote_data.thr & 0xFF;
    tx_buf[5] = ( remote_data.yaw >> 8 ) & 0xFF;
    tx_buf[6] = remote_data.yaw & 0xFF;
    tx_buf[7] = (remote_data.pit >> 8) & 0xFF;
    tx_buf[8] = remote_data.pit & 0xFF;
    tx_buf[9] = (remote_data.rol >> 8) & 0xFF;
    tx_buf[10] = remote_data.rol & 0xFF;

    //临界区保护
    taskENTER_CRITICAL();
    tx_buf[11] = remote_data.shutdown ;
    remote_data.shutdown = 0;
    tx_buf[12] = remote_data.fix_height ;
    remote_data.fix_height = 0;
    taskEXIT_CRITICAL();

    //计算校验和
    for(uint8_t i = 0; i < 13; i++)
    {
        check_sum += tx_buf[i];
    }
    tx_buf[13] = ( check_sum >> 24 ) & 0xFF;
    tx_buf[14] = ( check_sum >> 16 ) & 0xFF;
    tx_buf[15] = ( check_sum >> 8 ) & 0xFF;
    tx_buf[16] = check_sum & 0xFF;

    //检查发送结果，接受回传数据
    uint8_t ret = Int_SI24R1_TxPacket(tx_buf);

    if(ret == 0)
    {
        uint32_t start_tick = HAL_GetTick();

        Int_SI24R1_RX_Mode();

        while(Int_SI24R1_RxPacket(rx_buf) != 0)
        {
            if((HAL_GetTick() - start_tick) >= 20)
            {
                return;
            }

            vTaskDelay(1);
        }

        if(App_validate_telemetry(rx_buf) != 0)
        {
            return;
        }

        bmp280_init_status = rx_buf[3];
        bmp280_altitude_dm = (int16_t)(((uint16_t)rx_buf[4] << 8) | rx_buf[5]);
            
    }
}
