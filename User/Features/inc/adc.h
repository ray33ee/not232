#ifndef _ADC_HARDWARE
#define _ADC_HARDWARE


#include "stdint.h"
#include "ch32v20x.h"

void adc_init(void);

uint16_t adc_read(uint8_t adc_channel);

#endif