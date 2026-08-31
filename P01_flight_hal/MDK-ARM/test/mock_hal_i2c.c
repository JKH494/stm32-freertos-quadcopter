/**
 * @file mock_hal_i2c.c
 * @brief Recording mock implementations for HAL_I2C_Mem_Write / HAL_I2C_Mem_Read.
 */
#include "mock_hal_i2c.h"
#include <string.h>

/* --- Global instances --- */
MockI2CState       mock_i2c;
I2C_HandleTypeDef  hi2c1;

/* --- Init / reset --- */

void MockI2C_Reset(void)
{
    memset(&mock_i2c, 0, sizeof(mock_i2c));
    mock_i2c.write_return = HAL_OK;
    mock_i2c.read_return  = HAL_OK;
}

void MockI2C_SetWriteReturn(HAL_StatusTypeDef status)
{
    mock_i2c.write_return = status;
}

void MockI2C_SetReadReturn(HAL_StatusTypeDef status)
{
    mock_i2c.read_return = status;
}

void MockI2C_SetReadData(uint8_t reg, uint8_t data)
{
    mock_i2c.read_data_map[reg] = data;
    mock_i2c.read_data_set[reg] = 1;
}

/* --- Call inspection --- */

MockI2CCall *MockI2C_GetCall(int index)
{
    if (index < 0 || index >= mock_i2c.call_count)
        return (MockI2CCall *)0;
    return &mock_i2c.calls[index];
}

int MockI2C_GetCallCount(void)
{
    return mock_i2c.call_count;
}

int MockI2C_GetWriteCallCount(void)
{
    int count = 0;
    for (int i = 0; i < mock_i2c.call_count; i++)
        if (mock_i2c.calls[i].type == MOCK_CALL_WRITE)
            count++;
    return count;
}

int MockI2C_GetReadCallCount(void)
{
    int count = 0;
    for (int i = 0; i < mock_i2c.call_count; i++)
        if (mock_i2c.calls[i].type == MOCK_CALL_READ)
            count++;
    return count;
}

MockI2CCall *MockI2C_FindWrite(uint16_t reg)
{
    for (int i = 0; i < mock_i2c.call_count; i++)
        if (mock_i2c.calls[i].type == MOCK_CALL_WRITE &&
            mock_i2c.calls[i].mem_address == reg)
            return &mock_i2c.calls[i];
    return (MockI2CCall *)0;
}

MockI2CCall *MockI2C_FindRead(uint16_t reg)
{
    for (int i = 0; i < mock_i2c.call_count; i++)
        if (mock_i2c.calls[i].type == MOCK_CALL_READ &&
            mock_i2c.calls[i].mem_address == reg)
            return &mock_i2c.calls[i];
    return (MockI2CCall *)0;
}

int MockI2C_GetWrittenByte(uint16_t reg)
{
    MockI2CCall *c = MockI2C_FindWrite(reg);
    if (!c)
        return -1;
    return (int)c->data;
}

/* --- Mocked HAL functions --- */

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
                                    uint16_t DevAddress,
                                    uint16_t MemAddress,
                                    uint16_t MemAddSize,
                                    uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout)
{
    (void)hi2c;

    if (mock_i2c.call_count < MOCK_MAX_CALLS) {
        MockI2CCall *c = &mock_i2c.calls[mock_i2c.call_count];
        c->type         = MOCK_CALL_WRITE;
        c->dev_address  = DevAddress;
        c->mem_address  = MemAddress;
        c->mem_add_size = MemAddSize;
        c->data         = (Size > 0 && pData) ? pData[0] : 0;
        c->size         = Size;
        c->timeout      = Timeout;
        mock_i2c.call_count++;
    }
    return mock_i2c.write_return;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c,
                                   uint16_t DevAddress,
                                   uint16_t MemAddress,
                                   uint16_t MemAddSize,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout)
{
    (void)hi2c;
    (void)Timeout;

    if (mock_i2c.call_count < MOCK_MAX_CALLS) {
        MockI2CCall *c = &mock_i2c.calls[mock_i2c.call_count];
        c->type         = MOCK_CALL_READ;
        c->dev_address  = DevAddress;
        c->mem_address  = MemAddress;
        c->mem_add_size = MemAddSize;
        c->size         = Size;
        c->timeout      = Timeout;
        mock_i2c.call_count++;
    }

    /* Fill caller's buffer with configured data (default 0) */
    if (pData && Size > 0) {
        if (mock_i2c.read_data_set[MemAddress & 0xFF])
            pData[0] = mock_i2c.read_data_map[MemAddress & 0xFF];
        else
            pData[0] = 0;
    }

    return mock_i2c.read_return;
}

/* Stub for MX_I2C1_Init (not called in tests but linked) */
void MX_I2C1_Init(void)
{
    /* no-op in test environment */
}
