################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/USB_Serial/src/ch32v20x_usbfs_device.c \
../User/USB_Serial/src/usb_desc.c \
../User/USB_Serial/src/usb_serial.c 

C_DEPS += \
./User/USB_Serial/src/ch32v20x_usbfs_device.d \
./User/USB_Serial/src/usb_desc.d \
./User/USB_Serial/src/usb_serial.d 

OBJS += \
./User/USB_Serial/src/ch32v20x_usbfs_device.o \
./User/USB_Serial/src/usb_desc.o \
./User/USB_Serial/src/usb_serial.o 

DIR_OBJS += \
./User/USB_Serial/src/*.o \

DIR_DEPS += \
./User/USB_Serial/src/*.d \

DIR_EXPANDS += \
./User/USB_Serial/src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/USB_Serial/src/%.o: ../User/USB_Serial/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -fdump-rtl-expand -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"e:/Software Projects/moun_projects/not232/Debug" -I"e:/Software Projects/moun_projects/not232/Core" -I"e:/Software Projects/moun_projects/not232/User" -I"e:/Software Projects/moun_projects/not232/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

