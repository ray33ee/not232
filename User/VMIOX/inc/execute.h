#ifndef __EXECUTE_H
#define __EXECUTE_H

#include "state.h"
#include "opcodes.h"
#include "ch32v20x.h"
#include "USB_Serial/inc/usb_serial.h"
#include "debug.h"
#include "comms/inc/comms.h"
#include "Features/inc/gpio.h"

#define STACK_SIZE (16)

void run(uint8_t* opcodes, uint8_t* registers, uint32_t, uint8_t* pile_t, uint8_t* pile_r, uint32_t* tp, uint32_t* rp);

#endif