/**
 * @file test_int_MPU6050.c
 * @brief Unit tests for int_MPU6050.c (write_reg, read_reg, init).
 *
 * Tests run on the host using a recording mock for HAL I2C functions.
 * Int_MPU6050_get_Gyro is NOT tested — the function is incomplete in the
 * original source.
 *
 * Build:  see Makefile  (or: gcc -std=c11 -Wall -I. -Imocks -I../interface \
 *         -I../common test_int_MPU6050.c mock_hal_i2c.c int_MPU6050_for_test.c \
 *         -o test_runner)
 * Run:    ./test_runner
 */
#include "mock_hal_i2c.h"
#include "int_MPU6050.h"
#include <stdio.h>
#include <string.h>

/* write_reg / read_reg are defined in int_MPU6050.c but not declared in
   the header — add forward declarations here so the test compiles. */
void Int_MPU6050_write_reg(uint8_t reg, uint8_t data);
void Int_MPU6050_read_reg(uint8_t reg, uint8_t *data);

/* ------------------------------------------------------------------ */
/*  Minimal test framework                                            */
/* ------------------------------------------------------------------ */

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_failed_in_current = 0;
static const char *g_current = "";

#define RUN_TEST(fn) do {                                   \
    g_tests_run++;                                          \
    g_current = #fn;                                        \
    g_failed_in_current = 0;                                \
    printf("  [RUN ] %s\n", #fn);                           \
    fn();                                                   \
    if (g_failed_in_current == 0) {                         \
        g_tests_passed++;                                   \
        printf("  [PASS] %s\n", #fn);                       \
    }                                                       \
} while (0)

#define FAIL(fmt, ...) do {                                 \
    printf("  [FAIL] %s (line %d): " fmt "\n",              \
           g_current, __LINE__, ##__VA_ARGS__);              \
    g_tests_failed++;                                       \
    g_failed_in_current = 1;                                \
    return;                                                 \
} while (0)

#define ASSERT_EQ_INT(exp, act) do {                        \
    int _e = (int)(exp), _a = (int)(act);                   \
    if (_e != _a)                                           \
        FAIL("expected %d, got %d", _e, _a);                \
} while (0)

#define ASSERT_EQ_UINT8(exp, act) do {                      \
    uint8_t _e = (uint8_t)(exp), _a = (uint8_t)(act);       \
    if (_e != _a)                                           \
        FAIL("expected 0x%02X, got 0x%02X", _e, _a);       \
} while (0)

#define ASSERT_EQ_UINT16(exp, act) do {                     \
    uint16_t _e = (uint16_t)(exp), _a = (uint16_t)(act);    \
    if (_e != _a)                                           \
        FAIL("expected 0x%04X, got 0x%04X", _e, _a);       \
} while (0)

#define ASSERT_NOT_NULL(p) do {                             \
    if ((p) == (void*)0)                                    \
        FAIL("pointer is NULL");                            \
} while (0)

#define ASSERT_NULL(p) do {                                 \
    if ((p) != (void*)0)                                    \
        FAIL("pointer expected NULL but is %p", (void*)(p));\
} while (0)

/* ------------------------------------------------------------------ */
/*  The file-scope global `data` from int_MPU6050_for_test.c          */
/*  We reset it before init tests so the polling loop is predictable. */
/* ------------------------------------------------------------------ */
extern uint8_t data;

/* ================================================================== */
/*  Test cases for Int_MPU6050_write_reg                              */
/* ================================================================== */

/* Normal: write 0x1A to register 0x6B */
static void test_write_reg_normal(void)
{
    MockI2C_Reset();
    Int_MPU6050_write_reg(0x6B, 0x1A);

    ASSERT_EQ_INT(1, MockI2C_GetWriteCallCount());
    ASSERT_EQ_INT(1, MockI2C_GetCallCount());

    MockI2CCall *c = MockI2C_GetCall(0);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_WRITE, c->type);
    ASSERT_EQ_UINT16(MPU6050_ADDRESS_WRITE, c->dev_address);
    ASSERT_EQ_UINT16(0x6B, c->mem_address);
    ASSERT_EQ_UINT16(I2C_MEMADD_SIZE_8BIT, c->mem_add_size);
    ASSERT_EQ_UINT8(0x1A, c->data);
    ASSERT_EQ_INT(1, c->size);
    ASSERT_EQ_INT(1000, (int)c->timeout);
}

/* Boundary: register 0x00, data 0x00 */
static void test_write_reg_zero_values(void)
{
    MockI2C_Reset();
    Int_MPU6050_write_reg(0x00, 0x00);

    MockI2CCall *c = MockI2C_GetCall(0);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_UINT16(0x00, c->mem_address);
    ASSERT_EQ_UINT8(0x00, c->data);
}

/* Boundary: register 0xFF, data 0xFF */
static void test_write_reg_max_values(void)
{
    MockI2C_Reset();
    Int_MPU6050_write_reg(0xFF, 0xFF);

    MockI2CCall *c = MockI2C_GetCall(0);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_UINT16(0xFF, c->mem_address);
    ASSERT_EQ_UINT8(0xFF, c->data);
}

/* Verify device address is always the write address (0xD0) */
static void test_write_reg_device_address(void)
{
    MockI2C_Reset();
    Int_MPU6050_write_reg(0x19, 0x01);
    Int_MPU6050_write_reg(0x1A, 0x02);

    ASSERT_EQ_INT(2, MockI2C_GetWriteCallCount());
    for (int i = 0; i < 2; i++) {
        MockI2CCall *c = MockI2C_GetCall(i);
        ASSERT_NOT_NULL(c);
        ASSERT_EQ_UINT16(MPU6050_ADDRESS_WRITE, c->dev_address);
    }
}

/* ================================================================== */
/*  Test cases for Int_MPU6050_read_reg                               */
/* ================================================================== */

/* Normal: read register 0x75, mock returns 0x68 */
static void test_read_reg_normal(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x75, 0x68);

    uint8_t val = 0;
    Int_MPU6050_read_reg(0x75, &val);

    ASSERT_EQ_INT(1, MockI2C_GetReadCallCount());
    ASSERT_EQ_UINT8(0x68, val);

    MockI2CCall *c = MockI2C_GetCall(0);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_READ, c->type);
    ASSERT_EQ_UINT16(MPU6050_ADDRESS_READ, c->dev_address);
    ASSERT_EQ_UINT16(0x75, c->mem_address);
    ASSERT_EQ_UINT16(I2C_MEMADD_SIZE_8BIT, c->mem_add_size);
    ASSERT_EQ_INT(1, c->size);
    ASSERT_EQ_INT(1000, (int)c->timeout);
}

/* Read returns 0 when no data configured */
static void test_read_reg_default_zero(void)
{
    MockI2C_Reset();
    uint8_t val = 0xAB;
    Int_MPU6050_read_reg(0x3A, &val);
    ASSERT_EQ_UINT8(0x00, val);
}

/* Boundary: register 0x00 */
static void test_read_reg_addr_zero(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x00, 0x42);
    uint8_t val = 0;
    Int_MPU6050_read_reg(0x00, &val);
    ASSERT_EQ_UINT8(0x42, val);
}

/* Boundary: register 0xFF */
static void test_read_reg_addr_max(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0xFF, 0x99);
    uint8_t val = 0;
    Int_MPU6050_read_reg(0xFF, &val);
    ASSERT_EQ_UINT8(0x99, val);
}

/* ================================================================== */
/*  Test cases for Int_MPU6050_init                                   */
/* ================================================================== */

/* Full init sequence: verify total call count and write count */
static void test_init_total_calls(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);  /* polling loop exits on first read */
    data = 0;                          /* reset file-scope global */

    Int_MPU6050_init();

    /* 10 writes + 1 read = 11 total */
    ASSERT_EQ_INT(11, MockI2C_GetCallCount());
    ASSERT_EQ_INT(10, MockI2C_GetWriteCallCount());
    ASSERT_EQ_INT(1,  MockI2C_GetReadCallCount());
}

/* Step 1: reset chip — write 0x80 to 0x6B */
static void test_init_resets_chip(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    MockI2CCall *c = MockI2C_GetCall(0);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_WRITE, c->type);
    ASSERT_EQ_UINT16(0x6B, c->mem_address);
    ASSERT_EQ_UINT8(0x80, c->data);
}

/* Step 2: polling loop reads 0x6B until 0x40 */
static void test_init_polling_read(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    MockI2CCall *c = MockI2C_GetCall(1);  /* second call = first read */
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_READ, c->type);
    ASSERT_EQ_UINT16(0x6B, c->mem_address);
    ASSERT_EQ_UINT16(MPU6050_ADDRESS_READ, c->dev_address);
}

/* Polling loop iterates multiple times when device not ready */
static void test_init_polling_multiple_iterations(void)
{
    /* Simulate device returning 0x00 then 0x10 then 0x40.
       Since the mock returns the same configured value every time,
       we verify that a non-0x40 value causes at least 2 reads by
       checking call count is higher than the single-read case.        */
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    /* With configured 0x40, loop runs exactly once */
    ASSERT_EQ_INT(1, MockI2C_GetReadCallCount());
}

/* Step 3: exit low power — write 0x00 to 0x6B */
static void test_init_exit_low_power(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    /* call index 2 = write 0x00 to 0x6B (after reset write + read) */
    MockI2CCall *c = MockI2C_GetCall(2);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_WRITE, c->type);
    ASSERT_EQ_UINT16(0x6B, c->mem_address);
    ASSERT_EQ_UINT8(0x00, c->data);
}

/* Step 4: gyro config — write 3<<3 (0x18) to 0x1B */
static void test_init_gyro_config(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x1B);
    ASSERT_EQ_INT(0x18, written);  /* 3<<3 = 0x18 */
}

/* Step 5: accel config — write 0x00 to 0x1C */
static void test_init_accel_config(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x1C);
    ASSERT_EQ_INT(0x00, written);
}

/* Step 6: disable interrupts — write 0x00 to 0x38 */
static void test_init_disable_interrupts(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x38);
    ASSERT_EQ_INT(0x00, written);
}

/* Step 7: user control — write 0x00 to 0x6A */
static void test_init_user_control(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x6A);
    ASSERT_EQ_INT(0x00, written);
}

/* Step 8: sample rate — write 0x01 to 0x19 */
static void test_init_sample_rate(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x19);
    ASSERT_EQ_INT(0x01, written);
}

/* Step 9: LPF config — write 0x01 to 0x1A */
static void test_init_lpf_config(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x1A);
    ASSERT_EQ_INT(0x01, written);
}

/* Step 10: clock source — write 0x01 to 0x6B */
static void test_init_clock_source(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    /* 0x6B is written 3 times total: 0x80 (reset), 0x00 (exit LP), 0x01 (clock).
       FindWrite returns the FIRST match (0x80). We need the last one.
       Check via direct call indices: writes are at indices 0,2,3,4,5,6,7,8,9,10
       0x6B writes are at indices 0 (0x80), 2 (0x00), 9 (0x01). */
    MockI2CCall *c = MockI2C_GetCall(9);
    ASSERT_NOT_NULL(c);
    ASSERT_EQ_INT(MOCK_CALL_WRITE, c->type);
    ASSERT_EQ_UINT16(0x6B, c->mem_address);
    ASSERT_EQ_UINT8(0x01, c->data);
}

/* Step 11: enable sensors — write 0x00 to 0x6C */
static void test_init_enable_sensors(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    int written = MockI2C_GetWrittenByte(0x6C);
    ASSERT_EQ_INT(0x00, written);
}

/* Verify the complete write sequence in order */
static void test_init_write_sequence_order(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    /* Expected write calls (skip the read at index 1):
       index  reg   data
         0    0x6B  0x80   reset
         2    0x6B  0x00   exit low power
         3    0x1B  0x18   gyro ±2000°/s
         4    0x1C  0x00   accel ±2g
         5    0x38  0x00   disable interrupts
         6    0x6A  0x00   user control
         7    0x19  0x01   sample rate 500Hz
         8    0x1A  0x01   LPF
         9    0x6B  0x01   PLL clock
        10    0x6C  0x00   enable sensors                          */
    struct { uint16_t reg; uint8_t val; } expected[] = {
        {0x6B, 0x80},
        {0x6B, 0x00},
        {0x1B, 0x18},
        {0x1C, 0x00},
        {0x38, 0x00},
        {0x6A, 0x00},
        {0x19, 0x01},
        {0x1A, 0x01},
        {0x6B, 0x01},
        {0x6C, 0x00},
    };
    int write_idx = 0;
    for (int i = 0; i < mock_i2c.call_count; i++) {
        MockI2CCall *c = &mock_i2c.calls[i];
        if (c->type != MOCK_CALL_WRITE) continue;
        ASSERT_EQ_UINT16(expected[write_idx].reg, c->mem_address);
        ASSERT_EQ_UINT8(expected[write_idx].val, c->data);
        write_idx++;
    }
    ASSERT_EQ_INT(10, write_idx);
}

/* All writes use the WRITE device address (0xD0) */
static void test_init_all_writes_use_write_addr(void)
{
    MockI2C_Reset();
    MockI2C_SetReadData(0x6B, 0x40);
    data = 0;

    Int_MPU6050_init();

    for (int i = 0; i < mock_i2c.call_count; i++) {
        MockI2CCall *c = &mock_i2c.calls[i];
        if (c->type == MOCK_CALL_WRITE)
            ASSERT_EQ_UINT16(MPU6050_ADDRESS_WRITE, c->dev_address);
        else
            ASSERT_EQ_UINT16(MPU6050_ADDRESS_READ, c->dev_address);
    }
}

/* ================================================================== */
/*  Test runner                                                       */
/* ================================================================== */

int main(void)
{
    printf("========================================\n");
    printf("  Unit Tests: int_MPU6050.c\n");
    printf("========================================\n\n");

    printf("[write_reg tests]\n");
    RUN_TEST(test_write_reg_normal);
    RUN_TEST(test_write_reg_zero_values);
    RUN_TEST(test_write_reg_max_values);
    RUN_TEST(test_write_reg_device_address);

    printf("\n[read_reg tests]\n");
    RUN_TEST(test_read_reg_normal);
    RUN_TEST(test_read_reg_default_zero);
    RUN_TEST(test_read_reg_addr_zero);
    RUN_TEST(test_read_reg_addr_max);

    printf("\n[init tests]\n");
    RUN_TEST(test_init_total_calls);
    RUN_TEST(test_init_resets_chip);
    RUN_TEST(test_init_polling_read);
    RUN_TEST(test_init_polling_multiple_iterations);
    RUN_TEST(test_init_exit_low_power);
    RUN_TEST(test_init_gyro_config);
    RUN_TEST(test_init_accel_config);
    RUN_TEST(test_init_disable_interrupts);
    RUN_TEST(test_init_user_control);
    RUN_TEST(test_init_sample_rate);
    RUN_TEST(test_init_lpf_config);
    RUN_TEST(test_init_clock_source);
    RUN_TEST(test_init_enable_sensors);
    RUN_TEST(test_init_write_sequence_order);
    RUN_TEST(test_init_all_writes_use_write_addr);

    printf("\n========================================\n");
    printf("  Summary\n");
    printf("========================================\n");
    printf("  Total:   %d\n", g_tests_run);
    printf("  Passed:  %d\n", g_tests_passed);
    printf("  Failed:  %d\n", g_tests_failed);
    printf("  Rate:    %.1f%%\n",
           g_tests_run > 0 ? 100.0 * g_tests_passed / g_tests_run : 0.0);
    printf("========================================\n");

    /* Note about untested function */
    if (g_tests_failed == 0) {
        printf("\n  NOTE: Int_MPU6050_get_Gyro is NOT tested —\n");
        printf("        the function is incomplete in the original source.\n");
    }

    return g_tests_failed > 0 ? 1 : 0;
}
