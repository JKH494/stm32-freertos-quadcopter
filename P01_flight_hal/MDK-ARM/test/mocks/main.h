/**
 * @file main.h
 * @brief Mock main.h for unit testing - provides minimal HAL type stubs.
 *
 * This file replaces the real STM32 HAL main.h during host-side unit tests.
 * It provides just enough type definitions for compiling int_MPU6050.c
 * without the full HAL/STM32 toolchain.
 */
#ifndef __MOCK_MAIN_H
#define __MOCK_MAIN_H

#include <stdint.h>

/* --- HAL status enumeration (matches stm32f1xx_hal_def.h) --- */
typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

/* --- Minimal I2C handle stub (contents unused by tests) --- */
typedef struct {
    void *Instance;
    void *Init;
    void *unused;
} I2C_HandleTypeDef;

/* --- I2C memory address size constants --- */
#define I2C_MEMADD_SIZE_8BIT   0x0001U
#define I2C_MEMADD_SIZE_16BIT  0x0010U

#endif /* __MOCK_MAIN_H */
