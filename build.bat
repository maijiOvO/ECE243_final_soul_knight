@echo off
title RISC-V DE1-SoC Project Builder
echo ========================================================
echo           RISC-V Game Project Build System
echo ========================================================

:: 1. 检查编译器是否存在
where riscv-none-elf-gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] Cannot find riscv-none-elf-gcc. 
    echo Please ensure the toolchain /bin folder is in your PATH.
    pause
    exit /b
)

echo [1/3] Cleaning old files...
if exist test.elf del test.elf

echo [2/3] Compiling and Linking...
echo (Target: DE1-SoC RISC-V RV32)

:: 核心编译命令
:: 使用了 -O3 优化, -Wall 警告全开, -nostartfiles 使用自定义 start.s
:: 使用 *.c 自动匹配子文件夹下所有源码
riscv-none-elf-gcc -O3 -march=rv32imf_zicsr_zicbom -mabi=ilp32f -nostartfiles ^
start.s ^
./game_logic/*.c ^
./hardware/*.c ^
./sources/*.c ^
-o test.elf -lm

:: 2. 检查编译是否成功
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build Failed! Please check the error messages above.
    pause
    exit /b
)

echo [3/3] Success! Generated: test.elf
echo --------------------------------------------------------
:: 显示文件大小，方便监控资源占用
for %%I in (test.elf) do echo ELF File Size: %%~zI bytes
echo ========================================================
echo Ready to upload to CPUlator.
pause