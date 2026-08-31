#ifndef __INT_BMP280_H
#define __INT_BMP280_H

#include <stdbool.h>

#define BMP280_ADDR                         (0x76U << 1)
#define BMP280_CHIP_ID_REG                  0xD0U
#define BMP280_RESET_REG                    0xE0U
#define BMP280_STATUS_REG                   0xF3U
#define BMP280_CTRL_MEAS_REG                0xF4U
#define BMP280_CONFIG_REG                   0xF5U
#define BMP280_PRESSURE_MSB_REG             0xF7U
#define BMP280_CALIB_DATA_REG               0x88U

#define BMP280_CHIP_ID                      0x58U
#define BMP280_RESET_VALUE                  0xB6U
#define BMP280_CALIB_DATA_LENGTH            24U
#define BMP280_DATA_FRAME_SIZE              6U
#define BMP280_I2C_TIMEOUT_MS               10U

#define BMP280_PRESSURE_OSR_8X              4U
#define BMP280_TEMPERATURE_OSR_16X          5U
#define BMP280_NORMAL_MODE                  3U
#define BMP280_CTRL_MEAS_VALUE              ((BMP280_TEMPERATURE_OSR_16X << 5) | \
                                             (BMP280_PRESSURE_OSR_8X << 2) | \
                                             BMP280_NORMAL_MODE)
#define BMP280_FILTER_16                     (4U << 2)
#define BMP280_SEA_LEVEL_PRESSURE_HPA        1013.25f

bool BMP280Init(void);
bool BMP280GetData(float *pressure,
                   float *temperature,
                   float *asl);

#endif
