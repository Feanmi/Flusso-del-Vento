################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/modules/acellerometer.c \
../Core/Src/modules/battery_sensor.c \
../Core/Src/modules/bluetooth.c \
../Core/Src/modules/malfunction.c \
../Core/Src/modules/motor.c \
../Core/Src/modules/ranging_sensor.c \
../Core/Src/modules/servo.c \
../Core/Src/modules/speedometers.c \
../Core/Src/modules/wifi.c 

OBJS += \
./Core/Src/modules/acellerometer.o \
./Core/Src/modules/battery_sensor.o \
./Core/Src/modules/bluetooth.o \
./Core/Src/modules/malfunction.o \
./Core/Src/modules/motor.o \
./Core/Src/modules/ranging_sensor.o \
./Core/Src/modules/servo.o \
./Core/Src/modules/speedometers.o \
./Core/Src/modules/wifi.o 

C_DEPS += \
./Core/Src/modules/acellerometer.d \
./Core/Src/modules/battery_sensor.d \
./Core/Src/modules/bluetooth.d \
./Core/Src/modules/malfunction.d \
./Core/Src/modules/motor.d \
./Core/Src/modules/ranging_sensor.d \
./Core/Src/modules/servo.d \
./Core/Src/modules/speedometers.d \
./Core/Src/modules/wifi.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/modules/%.o Core/Src/modules/%.su Core/Src/modules/%.cyclo: ../Core/Src/modules/%.c Core/Src/modules/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32L151xC -c -I../Core/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc -I../Drivers/STM32L1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L1xx/Include -I../Drivers/CMSIS/Include -I"C:/my_projects/CRACK/huy/car_main (2)/car_main/Drivers/VL53L0X" -I"C:/my_projects/CRACK/huy/car_main (2)/car_main/Drivers/VL53L0X/core" -I"C:/my_projects/CRACK/huy/car_main (2)/car_main/Drivers/VL53L0X/core/inc" -I"C:/my_projects/CRACK/huy/car_main (2)/car_main/Drivers/VL53L0X/platform" -I"C:/my_projects/CRACK/huy/car_main (2)/car_main/Drivers/VL53L0X/platform/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-modules

clean-Core-2f-Src-2f-modules:
	-$(RM) ./Core/Src/modules/acellerometer.cyclo ./Core/Src/modules/acellerometer.d ./Core/Src/modules/acellerometer.o ./Core/Src/modules/acellerometer.su ./Core/Src/modules/battery_sensor.cyclo ./Core/Src/modules/battery_sensor.d ./Core/Src/modules/battery_sensor.o ./Core/Src/modules/battery_sensor.su ./Core/Src/modules/bluetooth.cyclo ./Core/Src/modules/bluetooth.d ./Core/Src/modules/bluetooth.o ./Core/Src/modules/bluetooth.su ./Core/Src/modules/malfunction.cyclo ./Core/Src/modules/malfunction.d ./Core/Src/modules/malfunction.o ./Core/Src/modules/malfunction.su ./Core/Src/modules/motor.cyclo ./Core/Src/modules/motor.d ./Core/Src/modules/motor.o ./Core/Src/modules/motor.su ./Core/Src/modules/ranging_sensor.cyclo ./Core/Src/modules/ranging_sensor.d ./Core/Src/modules/ranging_sensor.o ./Core/Src/modules/ranging_sensor.su ./Core/Src/modules/servo.cyclo ./Core/Src/modules/servo.d ./Core/Src/modules/servo.o ./Core/Src/modules/servo.su ./Core/Src/modules/speedometers.cyclo ./Core/Src/modules/speedometers.d ./Core/Src/modules/speedometers.o ./Core/Src/modules/speedometers.su ./Core/Src/modules/wifi.cyclo ./Core/Src/modules/wifi.d ./Core/Src/modules/wifi.o ./Core/Src/modules/wifi.su

.PHONY: clean-Core-2f-Src-2f-modules

