# Makefile for int_MPU6050 unit tests
#
# Usage:
#   make        - compile the test runner
#   make run    - compile and run tests
#   make clean  - remove build artifacts
#
# Requires: gcc (MinGW on Windows, or any C compiler)

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g
INCLUDES = -I. -Imocks -I../interface -I../common

# Sources
TEST_SRC   = test_int_MPU6050.c
MOCK_SRC   = mock_hal_i2c.c
UNIT_SRC   = int_MPU6050_for_test.c

SRCS       = $(TEST_SRC) $(MOCK_SRC) $(UNIT_SRC)
TARGET     = test_runner

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
