/**
 * @file mock_hal_i2c.h
 * @brief Recording mock for STM32 HAL I2C memory read/write functions.
 *
 * The mock records every HAL_I2C_Mem_Write / HAL_I2C_Mem_Read call so that
 * test cases can assert on device address, register address, data, size and
 * timeout. Read behaviour is configurable per-register via MockI2C_SetReadData.
 */
#ifndef MOCK_HAL_I2C_H
#define MOCK_HAL_I2C_H

#include "main.h"
#include <stdint.h>

#define MOCK_MAX_CALLS 64

typedef enum {
    MOCK_CALL_WRITE,
    MOCK_CALL_READ
} MockCallType;

typedef struct {
    MockCallType type;
    uint16_t     dev_address;
    uint16_t     mem_address;
    uint16_t     mem_add_size;
    uint8_t      data;       /* write: byte sent; read: byte returned       */
    uint16_t     size;
    uint32_t     timeout;
} MockI2CCall;

typedef struct {
    MockI2CCall       calls[MOCK_MAX_CALLS];
    int               call_count;
    HAL_StatusTypeDef write_return;
    HAL_StatusTypeDef read_return;
    /* Configurable per-register read data (256-byte address space) */
    uint8_t           read_data_map[256];
    uint8_t           read_data_set[256]; /* 1 = value has been configured */
} MockI2CState;

/* Global mock state instance */
extern MockI2CState mock_i2c;

/* I2C handle instance (referenced by int_MPU6050.c via &hi2c1) */
extern I2C_HandleTypeDef hi2c1;

/* --- Control / inspection API --- */

void              MockI2C_Reset(void);
void              MockI2C_SetWriteReturn(HAL_StatusTypeDef status);
void              MockI2C_SetReadReturn(HAL_StatusTypeDef status);
void              MockI2C_SetReadData(uint8_t reg, uint8_t data);

MockI2CCall      *MockI2C_GetCall(int index);
int               MockI2C_GetCallCount(void);
int               MockI2C_GetWriteCallCount(void);
int               MockI2C_GetReadCallCount(void);

/* Find first write/read call targeting the given register address */
MockI2CCall      *MockI2C_FindWrite(uint16_t reg);
MockI2CCall      *MockI2C_FindRead(uint16_t reg);

/* Convenience: get the written data byte for a register (or -1 if not found) */
int               MockI2C_GetWrittenByte(uint16_t reg);

#endif /* MOCK_HAL_I2C_H */
