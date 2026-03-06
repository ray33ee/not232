#ifndef _PWM_HARDWARE
#define _PWM_HARDWARE


#include "stdint.h"
#include "ch32v20x.h"

void pwm_init();

void pwm_set_duty(uint8_t pin_number, uint8_t duty);

#endif