################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/trans/recv/src/recv.c 

C_DEPS += \
./User/trans/recv/src/recv.d 

OBJS += \
./User/trans/recv/src/recv.o 

DIR_OBJS += \
./User/trans/recv/src/*.o \

DIR_DEPS += \
./User/trans/recv/src/*.d \

DIR_EXPANDS += \
./User/trans/recv/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/trans/recv/src/%.o: ../User/trans/recv/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

