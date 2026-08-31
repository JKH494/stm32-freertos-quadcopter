#include "int_bmp280.h"

#include <math.h>
#include <stdint.h>

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"


typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int32_t t_fine;
} BMP280_CalibData;

static BMP280_CalibData bmp280_calib;
static bool bmp280_initialized = false;

#define BMP280_SDA_GPIO_PORT       GPIOB
#define BMP280_SDA_GPIO_PIN        GPIO_PIN_10
#define BMP280_SCL_GPIO_PORT       GPIOB
#define BMP280_SCL_GPIO_PIN        GPIO_PIN_11
#define BMP280_I2C_DELAY_LOOPS     80U
#define BMP280_SCL_WAIT_LOOPS      100U

#define BMP280_SDA_HIGH() \
    HAL_GPIO_WritePin(BMP280_SDA_GPIO_PORT, BMP280_SDA_GPIO_PIN, GPIO_PIN_SET)
#define BMP280_SDA_LOW() \
    HAL_GPIO_WritePin(BMP280_SDA_GPIO_PORT, BMP280_SDA_GPIO_PIN, GPIO_PIN_RESET)
#define BMP280_SCL_HIGH() \
    HAL_GPIO_WritePin(BMP280_SCL_GPIO_PORT, BMP280_SCL_GPIO_PIN, GPIO_PIN_SET)
#define BMP280_SCL_LOW() \
    HAL_GPIO_WritePin(BMP280_SCL_GPIO_PORT, BMP280_SCL_GPIO_PIN, GPIO_PIN_RESET)
#define BMP280_READ_SDA() \
    HAL_GPIO_ReadPin(BMP280_SDA_GPIO_PORT, BMP280_SDA_GPIO_PIN)
#define BMP280_READ_SCL() \
    HAL_GPIO_ReadPin(BMP280_SCL_GPIO_PORT, BMP280_SCL_GPIO_PIN)

static void BMP280_I2C_Delay(void)
{
    volatile uint32_t i;

    for(i = 0U; i < BMP280_I2C_DELAY_LOOPS; i++)
    {
        __NOP();
    }
}

static bool BMP280_I2C_ReleaseSCL(void)
{
    uint16_t timeout = BMP280_SCL_WAIT_LOOPS;

    BMP280_SCL_HIGH();
    while(BMP280_READ_SCL() == GPIO_PIN_RESET)
    {
        if(timeout == 0U)
        {
            return false;
        }

        timeout--;
        BMP280_I2C_Delay();
    }

    BMP280_I2C_Delay();
    return true;
}

static bool BMP280_I2C_Start(void)
{
    BMP280_SDA_HIGH();
    BMP280_I2C_Delay();

    if(!BMP280_I2C_ReleaseSCL())
    {
        return false;
    }

    if(BMP280_READ_SDA() == GPIO_PIN_RESET)
    {
        BMP280_SCL_LOW();
        return false;
    }

    BMP280_SDA_LOW();
    BMP280_I2C_Delay();
    BMP280_SCL_LOW();
    BMP280_I2C_Delay();
    return true;
}

static void BMP280_I2C_Stop(void)
{
    BMP280_SDA_LOW();
    BMP280_I2C_Delay();

    if(BMP280_I2C_ReleaseSCL())
    {
        BMP280_SDA_HIGH();
        BMP280_I2C_Delay();
    }
}

static void BMP280_I2C_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint8_t pulse;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    //Set the output latch high before switching the pins to open drain.
    HAL_GPIO_WritePin(GPIOB,
                      BMP280_SDA_GPIO_PIN | BMP280_SCL_GPIO_PIN,
                      GPIO_PIN_SET);

    gpio.Pin = BMP280_SDA_GPIO_PIN | BMP280_SCL_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    BMP280_SDA_HIGH();
    BMP280_SCL_HIGH();
    BMP280_I2C_Delay();

    //Recover a slave that was reset in the middle of a transfer.
    if(BMP280_READ_SDA() == GPIO_PIN_RESET)
    {
        for(pulse = 0U; pulse < 9U; pulse++)
        {
            BMP280_SCL_LOW();
            BMP280_I2C_Delay();
            if(!BMP280_I2C_ReleaseSCL())
            {
                break;
            }
        }
    }

    BMP280_I2C_Stop();
}

static bool BMP280_I2C_WriteByte(uint8_t data)
{
    uint8_t bit;
    bool acknowledged;

    for(bit = 0U; bit < 8U; bit++)
    {
        if((data & 0x80U) != 0U)
        {
            BMP280_SDA_HIGH();
        }
        else
        {
            BMP280_SDA_LOW();
        }

        BMP280_I2C_Delay();
        if(!BMP280_I2C_ReleaseSCL())
        {
            return false;
        }

        BMP280_SCL_LOW();
        BMP280_I2C_Delay();
        data <<= 1;
    }

    BMP280_SDA_HIGH();
    BMP280_I2C_Delay();
    if(!BMP280_I2C_ReleaseSCL())
    {
        return false;
    }

    acknowledged = (BMP280_READ_SDA() == GPIO_PIN_RESET);
    BMP280_SCL_LOW();
    BMP280_I2C_Delay();
    return acknowledged;
}

static bool BMP280_I2C_ReadByte(uint8_t *data, bool send_ack)
{
    uint8_t bit;
    uint8_t value = 0U;

    BMP280_SDA_HIGH();

    for(bit = 0U; bit < 8U; bit++)
    {
        value <<= 1;
        if(!BMP280_I2C_ReleaseSCL())
        {
            return false;
        }

        if(BMP280_READ_SDA() == GPIO_PIN_SET)
        {
            value |= 0x01U;
        }

        BMP280_SCL_LOW();
        BMP280_I2C_Delay();
    }

    if(send_ack)
    {
        BMP280_SDA_LOW();
    }
    else
    {
        BMP280_SDA_HIGH();
    }

    BMP280_I2C_Delay();
    if(!BMP280_I2C_ReleaseSCL())
    {
        BMP280_SDA_HIGH();
        return false;
    }

    BMP280_SCL_LOW();
    BMP280_SDA_HIGH();
    BMP280_I2C_Delay();

    *data = value;
    return true;
}

static HAL_StatusTypeDef Int_BMP280_Read(uint8_t reg,
                                         uint8_t *data,
                                         uint16_t len)
{
    uint16_t i;
    HAL_StatusTypeDef status = HAL_ERROR;

    if((data == 0) || (len == 0U))
    {
        return HAL_ERROR;
    }

    taskENTER_CRITICAL();

    if(!BMP280_I2C_Start() ||
       !BMP280_I2C_WriteByte((uint8_t)BMP280_ADDR) ||
       !BMP280_I2C_WriteByte(reg) ||
       !BMP280_I2C_Start() ||
       !BMP280_I2C_WriteByte((uint8_t)(BMP280_ADDR | 0x01U)))
    {
        BMP280_I2C_Stop();
        taskEXIT_CRITICAL();
        return HAL_ERROR;
    }

    for(i = 0U; i < len; i++)
    {
        if(!BMP280_I2C_ReadByte(&data[i], i < (len - 1U)))
        {
            BMP280_I2C_Stop();
            taskEXIT_CRITICAL();
            return HAL_ERROR;
        }
    }

    BMP280_I2C_Stop();
    status = HAL_OK;

    taskEXIT_CRITICAL();
    return status;
}

static HAL_StatusTypeDef Int_BMP280_Write(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    taskENTER_CRITICAL();

    if(!BMP280_I2C_Start() ||
       !BMP280_I2C_WriteByte((uint8_t)BMP280_ADDR) ||
       !BMP280_I2C_WriteByte(reg) ||
       !BMP280_I2C_WriteByte(value))
    {
        BMP280_I2C_Stop();
        taskEXIT_CRITICAL();
        return HAL_ERROR;
    }

    BMP280_I2C_Stop();
    status = HAL_OK;

    taskEXIT_CRITICAL();
    return status;
}

static uint16_t BMP280_ReadU16LE(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t BMP280_ReadS16LE(const uint8_t *data)
{
    return (int16_t)BMP280_ReadU16LE(data);
}

static void BMP280_ParseCalibration(const uint8_t *data)
{
    bmp280_calib.dig_T1 = BMP280_ReadU16LE(&data[0]);
    bmp280_calib.dig_T2 = BMP280_ReadS16LE(&data[2]);
    bmp280_calib.dig_T3 = BMP280_ReadS16LE(&data[4]);
    bmp280_calib.dig_P1 = BMP280_ReadU16LE(&data[6]);
    bmp280_calib.dig_P2 = BMP280_ReadS16LE(&data[8]);
    bmp280_calib.dig_P3 = BMP280_ReadS16LE(&data[10]);
    bmp280_calib.dig_P4 = BMP280_ReadS16LE(&data[12]);
    bmp280_calib.dig_P5 = BMP280_ReadS16LE(&data[14]);
    bmp280_calib.dig_P6 = BMP280_ReadS16LE(&data[16]);
    bmp280_calib.dig_P7 = BMP280_ReadS16LE(&data[18]);
    bmp280_calib.dig_P8 = BMP280_ReadS16LE(&data[20]);
    bmp280_calib.dig_P9 = BMP280_ReadS16LE(&data[22]);
}

static bool BMP280_ReadRaw(int32_t *raw_pressure, int32_t *raw_temperature)
{
    uint8_t data[BMP280_DATA_FRAME_SIZE];

    if (Int_BMP280_Read(BMP280_PRESSURE_MSB_REG,
                        data,
                        BMP280_DATA_FRAME_SIZE) != HAL_OK)
    {
        return false;
    }

    *raw_pressure = (int32_t)(((uint32_t)data[0] << 12) |
                             ((uint32_t)data[1] << 4) |
                             ((uint32_t)data[2] >> 4));
    *raw_temperature = (int32_t)(((uint32_t)data[3] << 12) |
                                ((uint32_t)data[4] << 4) |
                                ((uint32_t)data[5] >> 4));
    return true;
}

static int32_t BMP280_CompensateTemperature(int32_t adc_temperature)
{
    int32_t var1;
    int32_t var2;

    var1 = (((adc_temperature >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1)) *
            (int32_t)bmp280_calib.dig_T2) >> 11;
    var2 = (((((adc_temperature >> 4) - (int32_t)bmp280_calib.dig_T1) *
              ((adc_temperature >> 4) - (int32_t)bmp280_calib.dig_T1)) >> 12) *
            (int32_t)bmp280_calib.dig_T3) >> 14;

    bmp280_calib.t_fine = var1 + var2;
    return (bmp280_calib.t_fine * 5 + 128) >> 8;
}

static uint32_t BMP280_CompensatePressure(int32_t adc_pressure)
{
    int64_t var1;
    int64_t var2;
    int64_t pressure;

    var1 = (int64_t)bmp280_calib.t_fine - 128000;
    var2 = var1 * var1 * (int64_t)bmp280_calib.dig_P6;
    var2 += (var1 * (int64_t)bmp280_calib.dig_P5) << 17;
    var2 += ((int64_t)bmp280_calib.dig_P4) << 35;
    var1 = ((var1 * var1 * (int64_t)bmp280_calib.dig_P3) >> 8) +
           ((var1 * (int64_t)bmp280_calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) *
            (int64_t)bmp280_calib.dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0U;
    }

    pressure = 1048576 - adc_pressure;
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)bmp280_calib.dig_P9 *
            (pressure >> 13) * (pressure >> 13)) >> 25;
    var2 = ((int64_t)bmp280_calib.dig_P8 * pressure) >> 19;
    pressure = ((pressure + var1 + var2) >> 8) +
               ((int64_t)bmp280_calib.dig_P7 << 4);

    return (uint32_t)pressure;
}

static float BMP280_PressureToAltitude(float pressure_hpa)
{
    if (pressure_hpa <= 0.0f)
    {
        return 0.0f;
    }

    return 44330.0f *
           (1.0f - powf(pressure_hpa / BMP280_SEA_LEVEL_PRESSURE_HPA,
                        0.19029495f));
}

bool BMP280Init(void)
{
    uint8_t chip_id;
    uint8_t status;
    uint8_t calibration[BMP280_CALIB_DATA_LENGTH];
    uint8_t retry;

    if (bmp280_initialized)
    {
        return true;
    }

    BMP280_I2C_Init();
    HAL_Delay(20U);

    if ((Int_BMP280_Read(BMP280_CHIP_ID_REG, &chip_id, 1U) != HAL_OK) ||
        (chip_id != BMP280_CHIP_ID))
    {
        return false;
    }

    if (Int_BMP280_Write(BMP280_RESET_REG, BMP280_RESET_VALUE) != HAL_OK)
    {
        return false;
    }

    HAL_Delay(5U);
    for (retry = 0U; retry < 10U; retry++)
    {
        if (Int_BMP280_Read(BMP280_STATUS_REG, &status, 1U) != HAL_OK)
        {
            return false;
        }

        if ((status & 0x01U) == 0U)
        {
            break;
        }

        HAL_Delay(2U);
    }

    if ((status & 0x01U) != 0U)
    {
        return false;
    }

    if (Int_BMP280_Read(BMP280_CALIB_DATA_REG,
                        calibration,
                        BMP280_CALIB_DATA_LENGTH) != HAL_OK)
    {
        return false;
    }

    BMP280_ParseCalibration(calibration);
    if (bmp280_calib.dig_P1 == 0U)
    {
        return false;
    }

    if (Int_BMP280_Write(BMP280_CONFIG_REG, BMP280_FILTER_16) != HAL_OK)
    {
        return false;
    }

    if (Int_BMP280_Write(BMP280_CTRL_MEAS_REG,
                         BMP280_CTRL_MEAS_VALUE) != HAL_OK)
    {
        return false;
    }

    bmp280_initialized = true;
    return true;
}

bool BMP280GetData(float *pressure, float *temperature, float *asl)
{
    int32_t raw_pressure;
    int32_t raw_temperature;
    int32_t temperature_x100;
    uint32_t pressure_q24_8;
    float pressure_hpa;

    if ((!bmp280_initialized) ||
        (pressure == 0) ||
        (temperature == 0) ||
        (asl == 0))
    {
        return false;
    }

    if (!BMP280_ReadRaw(&raw_pressure, &raw_temperature))
    {
        return false;
    }

    temperature_x100 = BMP280_CompensateTemperature(raw_temperature);
    pressure_q24_8 = BMP280_CompensatePressure(raw_pressure);
    if (pressure_q24_8 == 0U)
    {
        return false;
    }

    pressure_hpa = (float)pressure_q24_8 / 25600.0f;
    *temperature = (float)temperature_x100 / 100.0f;
    *pressure = pressure_hpa;
    *asl = BMP280_PressureToAltitude(pressure_hpa);
    return true;
}
