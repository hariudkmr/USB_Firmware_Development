################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/systeminit.c \
../Src/usb_driver.c \
../Src/usb_framework.c 

OBJS += \
./Src/main.o \
./Src/systeminit.o \
./Src/usb_driver.o \
./Src/usb_framework.o 

C_DEPS += \
./Src/main.d \
./Src/systeminit.d \
./Src/usb_driver.d \
./Src/usb_framework.d 


# Each subdirectory must supply rules for building sources it contributes
Src/main.o: ../Src/main.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407xx -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/Helpers" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Include" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/main.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/systeminit.o: ../Src/systeminit.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407xx -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/Helpers" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Include" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/systeminit.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/usb_driver.o: ../Src/usb_driver.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407xx -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/Helpers" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Include" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/usb_driver.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/usb_framework.o: ../Src/usb_framework.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407xx -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/Helpers" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Include" -I"/home/hari/STM32CubeIDE/workspace_1.6.1/USB_Project/Inc/CMSIS/Device/ST/STM32F4xx/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/usb_framework.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

