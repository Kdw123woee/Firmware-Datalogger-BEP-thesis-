################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.c 

OBJS += \
./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.o 

C_DEPS += \
./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32U083C-DK/%.o Drivers/BSP/STM32U083C-DK/%.su Drivers/BSP/STM32U083C-DK/%.cyclo: ../Drivers/BSP/STM32U083C-DK/%.c Drivers/BSP/STM32U083C-DK/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U083xx -c -I../Core/Inc -I../Drivers/STM32U0xx_HAL_Driver/Inc -I../Drivers/STM32U0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32U083C-2d-DK

clean-Drivers-2f-BSP-2f-STM32U083C-2d-DK:
	-$(RM) ./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.cyclo ./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.d ./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.o ./Drivers/BSP/STM32U083C-DK/stm32u083c_discovery.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32U083C-2d-DK

