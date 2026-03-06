#ifndef _PULSE_OUT_HARDSOFT
#define _PULSE_OUT_HARDSOFT

#include "stdint.h"
#include "ch32v20x.h"
#include "Features/inc/pwm.h"

void pulseio_out_send(uint8_t pin_number, uint8_t duty, uint16_t* pulses, uint32_t pulse_count);

#endif