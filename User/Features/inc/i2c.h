#ifndef _I2C_BITBANG
#define _I2C_BITBANG


#include "stdint.h"
#include "ch32v20x.h"

uint8_t i2c_scan_address(uint8_t sda_pin, uint8_t scl_pin, uint8_t address);

void i2c_send_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop);

void i2c_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop);

void i2c_send_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* out_buffer, uint32_t out_len, uint8_t* in_buffer, uint32_t in_len, int stop);

#endif