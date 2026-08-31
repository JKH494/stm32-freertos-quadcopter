/**
 * @file int_MPU6050_for_test.c
 * @brief TEST COPY of interface/int_MPU6050.c
 *
 * This file is an exact copy of the original source with ONE modification:
 * the incomplete Int_MPU6050_get_Gyro() function has been stubbed out so the
 * file compiles cleanly under the host test compiler.  The original function
 * body is truncated (ends with an incomplete `uint8_t` declaration) and would
 * cause a syntax error.
 *
 * All other functions are identical to the original source.
 *
 * NOTE: Int_MPU6050_get_Gyro is NOT tested because it is unfinished in the
 *       original code.  When the implementation is completed, re-generate
 *       tests for it.
 */
#include "int_MPU6050.h"

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
 * @param reg
 * @param data
 */
void Int_MPU6050_read_reg(uint8_t reg, uint8_t *data)
{
    //1，句柄 2，从机地址 3，寄存器地址 4，寄存器地址长度 5，存放读取数据的地址 6，读取数据个数 7，超时时间
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDRESS_READ, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
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
        Int_MPU6050_read_reg(0x6B, &data);
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
}

/* --- TEST-ONLY STUB: original function is incomplete and untested --- */
void Int_MPU6050_get_Gyro(Gyro_Struct *gyro)
{
    (void)gyro;
    /* TODO: original implementation is truncated — not testable yet */
}
