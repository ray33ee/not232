#ifndef _GPIO_HARDWARE
#define _GPIO_HARDWARE


#include "stdint.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x.h"
#include "debug.h"

//Change A0-A7, D0-D7, F0-F3 to floating input
void gpio_init_default();

//Setup pins A0-A7 and D0-7 (pin numbers 0-15, CH32 numbers PA0-PA15)
void gpio_init_ad_pins(uint8_t pin, GPIOMode_TypeDef mode);

//Setup pins F0-F3 (pin numbers 16-19, CH32 numbers PB0, PB1, PB14 and PB15)
void gpio_init_f_pins(uint8_t pin, GPIOMode_TypeDef mode);

void gpio_init_adf_pins(uint8_t pin, GPIOMode_TypeDef mode);

uint32_t gpio_read_adf_pin(uint8_t pin);

void gpio_set_adf_pin(uint8_t pin);

void gpio_clear_adf_pin(uint8_t pin);

#endif