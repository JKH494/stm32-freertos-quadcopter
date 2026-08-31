#include "int_MPU6050.h"

//偏移值
int32_t offset_Gyro_X = 0;
int32_t offset_Gyro_Y = 0;
int32_t offset_Gyro_Z = 0;
int32_t offset_Accel_X = 0;
int32_t offset_Accel_Y = 0;
int32_t offset_Accel_Z = 0;

/**
 * @brief 写寄存器
 * 
 * @param reg 寄存器地址
 * @param data 写入数据
 */
void Int_MPU6050_write_reg(uint8_t reg, uint8_t data)
{
    //1，句柄 2，从机地址 3，寄存器地址 4，寄存器地址长度 5，数据指针 6，传输个数 7，超时时间
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDRESS_WRITE, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}
/**
 * @brief 读寄存器  
 * 
 * @param reg 寄存器地址
 * @param data 存放读取数据的指针
 * @param length 读取数据的长度
 */
void Int_MPU6050_read_reg(uint8_t reg, uint8_t *data, uint8_t length)
{
    //1，句柄 2，从机地址 3，寄存器地址 4，寄存器地址长度 5，存放读取数据的地址 6，读取数据个数 7，超时时间
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDRESS_READ, reg, I2C_MEMADD_SIZE_8BIT, data, length, 1000);
}

/**
 * @brief 零偏校准
 * 
 */
void Int_MPU6050_calculate_offset(void)
{
    //等待飞机平稳
    Accel_Struct current_accel = {0};
    Accel_Struct last_accel = {0};
    uint8_t count = 0;
    //100次采样，每次加速度X Y Z轴的误差不超过200
    Int_MPU6050_get_Acc(&last_accel);
    while(count < 100)
    {
        Int_MPU6050_get_Acc(&current_accel);
        if(abs(current_accel.Accel_X - last_accel.Accel_X) < 400 && abs(current_accel.Accel_Y - last_accel.Accel_Y) < 400 
            && abs(current_accel.Accel_Z - last_accel.Accel_Z) < 400)
        {
            count++;
        }
        else
        {
            count = 0;
        }
        last_accel = current_accel;
        vTaskDelay(6);
    }

    //飞机已经停稳，开始计算偏移值
    Gyro_Accel_Struct gyro_accel_data = {0};
    int32_t sum_gyro_x = 0;
    int32_t sum_gyro_y = 0;
    int32_t sum_gyro_z = 0;
    int32_t sum_accel_x = 0;
    int32_t sum_accel_y = 0;
    int32_t sum_accel_z = 0;

    offset_Gyro_X = 0;
    offset_Gyro_Y = 0;
    offset_Gyro_Z = 0;
    offset_Accel_X = 0;
    offset_Accel_Y = 0;
    offset_Accel_Z = 0;

    for(uint8_t i = 0; i < 100; i++)
    {
        Int_MPU6050_get_Data(&gyro_accel_data);
        sum_gyro_x += gyro_accel_data.gyro.Gyro_X;
        sum_gyro_y += gyro_accel_data.gyro.Gyro_Y;
        sum_gyro_z += gyro_accel_data.gyro.Gyro_Z;
        sum_accel_x += gyro_accel_data.accel.Accel_X;
        sum_accel_y += gyro_accel_data.accel.Accel_Y;
        sum_accel_z += (gyro_accel_data.accel.Accel_Z - 16384);

        vTaskDelay(6);
    }
    //计算平均值
    offset_Gyro_X = sum_gyro_x / 100;
    offset_Gyro_Y = sum_gyro_y / 100;
    offset_Gyro_Z = sum_gyro_z / 100;
    offset_Accel_X = sum_accel_x / 100;
    offset_Accel_Y = sum_accel_y / 100;
    offset_Accel_Z = sum_accel_z / 100;
}

uint8_t data = 0;
/**
 * @brief 初始化MPU6050
 * 
 */
void Int_MPU6050_init(void)
{
    //重启芯片 => 重置寄存器 ，给电源管理寄存器写 1
    Int_MPU6050_write_reg(0x6B, 0x80);
    //循环读取电源管理寄存器，直到其值为 0x40，表示重启完成，进入了低功耗模式
    while( data != 0x40)
    {
        Int_MPU6050_read_reg(0x6B, &data,1);
    }
    //从低功耗模式进入正常工作模式
    Int_MPU6050_write_reg(0x6B, 0x00);

    //填写角速度量程为±2000°/s
    Int_MPU6050_write_reg(0x1B, 3<<3);
    //填写加速度量程为±2g
    Int_MPU6050_write_reg(0x1C, 0x00);
    //关闭中断使能
    Int_MPU6050_write_reg(0x38, 0x00);

    //用户配置寄存器，不使用FIFO队列，关闭MPU6050辅助I2C主机功能
    Int_MPU6050_write_reg(0x6A, 0x00);

    //设置采样频率 默认1000Hz 要求大于使用频率2倍，故设为500Hz   (2 - 1)
    Int_MPU6050_write_reg(0x19, 0x01);
    //设置低通滤波器值为 184 188hz
    Int_MPU6050_write_reg(0x1A, 0x01);
    //配置时用的系统时钟为使用PLL的
    Int_MPU6050_write_reg(0x6B, 0x01);
    //使能加速度和角速度传感器
    Int_MPU6050_write_reg(0x6C, 0x00);

    //进行零偏校准
    Int_MPU6050_calculate_offset();
}

void Int_MPU6050_get_Gyro(Gyro_Struct *gyro)
{                   
    //存储角速度的寄存器从0x43开始，高位在前，xyz的顺序
    uint8_t Buff_data[6] = {0};
    Int_MPU6050_read_reg(0x43,Buff_data,6);
    gyro->Gyro_X = (Buff_data[0] << 8 | Buff_data[1]) - offset_Gyro_X;
    gyro->Gyro_Y = (Buff_data[2] << 8 | Buff_data[3]) - offset_Gyro_Y;
    gyro->Gyro_Z = (Buff_data[4] << 8 | Buff_data[5]) - offset_Gyro_Z;    
}
void Int_MPU6050_get_Acc(Accel_Struct *acc)
{
    uint8_t Buff_data[6] = {0};
    Int_MPU6050_read_reg(0x3B,Buff_data,6);
    acc->Accel_X = (Buff_data[0] << 8 | Buff_data[1]) - offset_Accel_X;
    acc->Accel_Y = (Buff_data[2] << 8 | Buff_data[3]) - offset_Accel_Y;
    acc->Accel_Z = (Buff_data[4] << 8 | Buff_data[5]) - offset_Accel_Z;
}
void Int_MPU6050_get_Data(Gyro_Accel_Struct *data)
{
    Int_MPU6050_get_Gyro(&data->gyro);
    Int_MPU6050_get_Acc(&data->accel);
}

