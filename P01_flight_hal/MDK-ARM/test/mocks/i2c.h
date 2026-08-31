/**
 * @file i2c.h
 * @brief Mock i2c.h for unit testing.
 *
 * Replaces the real i2c.h so that int_MPU6050.c can compile in the test
 * environment. Declares the extern handle and HAL I2C function prototypes
 * whose implementations are provided by mock_hal_i2c.c.
 */
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Extern I2C handle (instance lives in mock_hal_i2c.c) */
extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void);

/* HAL I2C memory access prototypes (mocked in mock_hal_i2c.c) */
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
                                    uint16_t DevAddress,
                                    uint16_t MemAddress,
                                    uint16_t MemAddSize,
                                    uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout);

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c,
                                   uint16_t DevAddress,
                                   uint16_t MemAddress,
                                   uint16_t MemAddSize,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
