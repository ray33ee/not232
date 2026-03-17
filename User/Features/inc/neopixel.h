#ifndef _NEOPIXEL_BITBANG
#define _NEOPIXEL_BITBANG


#include "stdint.h"
#include "ch32v20x.h"

void neopixel_send_buffer(GPIO_TypeDef* port, uint16_t pin_mask, uint8_t* buffer, uint32_t len);

#endif