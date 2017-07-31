################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/onewireio.c \
../Src/owcompletesearch.c \
../Src/search.c \
../Src/stm32l0xx_hal_msp.c \
../Src/stm32l0xx_it.c \
../Src/system_stm32l0xx.c 

OBJS += \
./Src/main.o \
./Src/onewireio.o \
./Src/owcompletesearch.o \
./Src/search.o \
./Src/stm32l0xx_hal_msp.o \
./Src/stm32l0xx_it.o \
./Src/system_stm32l0xx.o 

C_DEPS += \
./Src/main.d \
./Src/onewireio.d \
./Src/owcompletesearch.d \
./Src/search.d \
./Src/stm32l0xx_hal_msp.d \
./Src/stm32l0xx_it.d \
./Src/system_stm32l0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32L053xx -I"C:/Users/user/workspace/uart_test/Inc" -I"C:/Users/user/workspace/uart_test/Drivers/STM32L0xx_HAL_Driver/Inc" -I"C:/Users/user/workspace/uart_test/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy" -I"C:/Users/user/workspace/uart_test/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/user/workspace/uart_test/Drivers/CMSIS/Include" -I"C:/Users/user/workspace/uart_test/Inc"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


