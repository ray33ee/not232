#ifndef _SPI_BITBANG
#define _SPI_BITBANG


#include "stdint.h"
#include "ch32v20x.h"
#include "Features/inc/gpio.h"
#include "ch32v20x.h"

void spi_init(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin);

void spi_read(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint8_t write_value, uint32_t delay, uint8_t* buffer, uint32_t len);

void spi_write(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint32_t delay, uint8_t* buffer, uint32_t len);

void spi_write_read(uint8_t sck_pin, uint8_t mosi_pin, uint8_t miso_pin, uint8_t mode, uint32_t delay, uint8_t* out_buffer, uint8_t* in_buffer, uint32_t len);

#endif