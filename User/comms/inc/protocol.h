#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "stdint.h"
#include "USB_Serial/inc/usb_serial.h"
#include "USB_Serial/inc/ch32v20x_usbfs_device.h"
#include "comms/inc/i2c_device.h"

#define COMMS_MOVE_USB  (1)
#define COMMS_MOVE_I2C  (2)

void comms_init(uint32_t mode, uint8_t i2c_address);

int comms_is_i2c_mode(void);

void comms_flush_i2c(void);

void comms_send_buffer(uint8_t* buffer, uint32_t size);

void comms_send_u8(uint8_t byte);

void comms_send_u32(uint32_t byte);

void comms_send_array(uint8_t* array, uint32_t size);

void comms_send_string(char* str);

void comms_recv_buffer(uint8_t* buffer, uint32_t size);

uint8_t comms_recv_u8();

uint32_t comms_recv_u32();

void comms_recv_array(uint8_t* array, uint32_t* size);

void comms_recv_string(uint8_t* str);

#endif