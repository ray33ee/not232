#ifndef _ONE_WIRE_BITBANG
#define _ONE_WIRE_BITBANG


#include "stdint.h"
#include "ch32v20x.h"

// Standard speed one wire delays as per https://www.analog.com/en/resources/technical-articles/1wire-communication-through-software.html
#define OW_DELAY_A              (6)
#define OW_DELAY_B              (64)
#define OW_DELAY_C              (60)
#define OW_DELAY_D              (10)
#define OW_DELAY_E              (9)
#define OW_DELAY_F              (55)
#define OW_DELAY_G              (0)
#define OW_DELAY_H              (480)
#define OW_DELAY_I              (70)
#define OW_DELAY_J              (410)

uint8_t one_wire_reset(uint16_t pin_mask);

void one_wire_write0(uint16_t pin_mask);

void one_wire_write1(uint16_t pin_mask);

uint8_t one_wire_read(uint16_t pin_mask);

#endif