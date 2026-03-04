################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/comms/src/comms.c 

C_DEPS += \
./User/comms/src/comms.d 

OBJS += \
./User/comms/src/comms.o 

DIR_OBJS += \
./User/comms/src/*.o \

DIR_DEPS += \
./User/comms/src/*.d \

DIR_EXPANDS += \
./User/comms/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/comms/src/%.o: ../User/comms/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

