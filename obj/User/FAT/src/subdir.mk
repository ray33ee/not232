################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/FAT/src/lfs.c \
../User/FAT/src/lfs_util.c 

C_DEPS += \
./User/FAT/src/lfs.d \
./User/FAT/src/lfs_util.d 

OBJS += \
./User/FAT/src/lfs.o \
./User/FAT/src/lfs_util.o 

DIR_OBJS += \
./User/FAT/src/*.o \

DIR_DEPS += \
./User/FAT/src/*.d \

DIR_EXPANDS += \
./User/FAT/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/FAT/src/%.o: ../User/FAT/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

