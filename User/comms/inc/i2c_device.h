#ifndef _I2C_DEVICE_H
#define _I2C_DEVICE_H

#include "stdint.h"
#include "ch32v20x.h"

void i2c_device_init(uint8_t address);

void i2c_device_read(uint8_t* buffer, uint32_t size);

void i2c_device_write(uint8_t* buffer, uint32_t size);

#endif