################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Features/src/adc.c \
../User/Features/src/gpio.c \
../User/Features/src/i2c.c \
../User/Features/src/neopixel.c \
../User/Features/src/one_wire.c \
../User/Features/src/pulse_in.c \
../User/Features/src/pulse_out.c \
../User/Features/src/pwm.c \
../User/Features/src/spi.c \
../User/Features/src/touchkey.c 

C_DEPS += \
./User/Features/src/adc.d \
./User/Features/src/gpio.d \
./User/Features/src/i2c.d \
./User/Features/src/neopixel.d \
./User/Features/src/one_wire.d \
./User/Features/src/pulse_in.d \
./User/Features/src/pulse_out.d \
./User/Features/src/pwm.d \
./User/Features/src/spi.d \
./User/Features/src/touchkey.d 

OBJS += \
./User/Features/src/adc.o \
./User/Features/src/gpio.o \
./User/Features/src/i2c.o \
./User/Features/src/neopixel.o \
./User/Features/src/one_wire.o \
./User/Features/src/pulse_in.o \
./User/Features/src/pulse_out.o \
./User/Features/src/pwm.o \
./User/Features/src/spi.o \
./User/Features/src/touchkey.o 

DIR_OBJS += \
./User/Features/src/*.o \

DIR_DEPS += \
./User/Features/src/*.d \

DIR_EXPANDS += \
./User/Features/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/Features/src/%.o: ../User/Features/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

