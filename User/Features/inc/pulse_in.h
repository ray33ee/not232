#ifndef _PULSE_IN_TIM
#define _PULSE_IN_TIM

#include "stdint.h"
#include "ch32v20x.h"
#include "USB_Serial/inc/usb_serial.h"

extern uint32_t pio_head;
extern uint32_t pio_count;
extern uint16_t pio_buffer[PIO_RING_SIZE];

void pulseio_in_init(void);

void pulseio_in_resume(uint8_t pin, uint32_t trigger_duration);

void pulseio_in_clear();

void pulseio_in_stop(uint8_t pin);

void pulseio_in_read(uint32_t try_read);

void pulseio_in_tim4_start(void);

uint16_t pulseio_in_tim4_stop(void);

#endif