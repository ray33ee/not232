################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/bitbang/src/i2c.c \
../User/bitbang/src/neopixel.c \
../User/bitbang/src/one_wire.c \
../User/bitbang/src/spi.c 

C_DEPS += \
./User/bitbang/src/i2c.d \
./User/bitbang/src/neopixel.d \
./User/bitbang/src/one_wire.d \
./User/bitbang/src/spi.d 

OBJS += \
./User/bitbang/src/i2c.o \
./User/bitbang/src/neopixel.o \
./User/bitbang/src/one_wire.o \
./User/bitbang/src/spi.o 

DIR_OBJS += \
./User/bitbang/src/*.o \

DIR_DEPS += \
./User/bitbang/src/*.d \

DIR_EXPANDS += \
./User/bitbang/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/bitbang/src/%.o: ../User/bitbang/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

