@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cl /std:c11 /utf-8 /W4 /Fe:test_runner.exe /I. /Imocks /I..\interface /I..\common test_int_MPU6050.c mock_hal_i2c.c int_MPU6050_for_test.c
