#ifndef _GPIO_HARDWARE
#define _GPIO_HARDWARE


#include "stdint.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x.h"
#include "debug.h"

#define PIN_COUNT (20)

//Change A0-A7, D0-D7, F0-F3 to floating input
void gpio_init_default();

void gpio_init_pin(uint8_t pin, GPIOMode_TypeDef mode);

uint32_t gpio_read_pin(uint8_t pin);

void gpio_set_pin(uint8_t pin);

void gpio_clear_pin(uint8_t pin);

#endif