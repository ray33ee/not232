################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/program/src/execute.c \
../User/program/src/opcodes.c 

C_DEPS += \
./User/program/src/execute.d \
./User/program/src/opcodes.d 

OBJS += \
./User/program/src/execute.o \
./User/program/src/opcodes.o 

DIR_OBJS += \
./User/program/src/*.o \

DIR_DEPS += \
./User/program/src/*.d \

DIR_EXPANDS += \
./User/program/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/program/src/%.o: ../User/program/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

