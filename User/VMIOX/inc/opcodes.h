#ifndef __OPCODES_H
#define __OPCODES_H

#include "stdint.h"
#include "comms/inc/comms.h"

/* NOP */
#define NOP                                     ((uint8_t)0x00)

/* IO */
#define SET_PIN_X                               ((uint8_t)0x01)
#define CLEAR_PIN_X                             ((uint8_t)0x02)

/* Shift IO*/
#define SHIFT_LEFT_IN_PIN_X_TO_REG_Y            ((uint8_t)0x03)
#define SHIFT_RIGHT_OUT_PIN_X_FROM_REG_Y        ((uint8_t)0x04)
#define SHIFT_LEFT_OUT_PIN_X_FROM_REG_Y         ((uint8_t)0x0C)
#define SHIFT_RIGHT_IN_PIN_X_TO_REG_Y           ((uint8_t)0x0D)

/* Conditional Blocking */
#define WAIT_FOR_LOW_PIN_X_TIMEOUT_Y            ((uint8_t)0x05)
#define WAIT_FOR_HIGH_PIN_X_TIMEOUT_Y           ((uint8_t)0x06)
#define WAIT_FOR_RISING_EDGE_X_TIMEOUT_Y        ((uint8_t)0x07)
#define WAIT_FOR_FALLING_EDGE_X_TIMEOUT_Y       ((uint8_t)0x08)

/* Timer blocking */
#define DELAY_NOPS_X                            ((uint8_t)0x24)
#define DELAY_TICKS_X                           ((uint8_t)0x09)
#define DELAY_MICROS_X                          ((uint8_t)0x0A)
#define DELAY_MILLIS_X                          ((uint8_t)0x0B)

/* Port IO */
#define WRITE_BANK_A_REG_X                      ((uint8_t)0x0E)
#define WRITE_BANK_D_REG_X                      ((uint8_t)0x0F)
#define READ_BANK_A_REG_X                       ((uint8_t)0x17)
#define READ_BANK_D_REG_X                       ((uint8_t)0x18)

/* Setup IO */
#define INPUT_FLOATING                          ((uint8_t)0x10)
#define INPUT_PULL_DOWN                         ((uint8_t)0x11)
#define INPUT_PULL_UP                           ((uint8_t)0x12)
#define OUTPUT_PP                               ((uint8_t)0x13)
#define OUTPUT_OD                               ((uint8_t)0x14)

/* Control Flow */
#define CALL_X                                  ((uint8_t)0x15)
#define RET                                     ((uint8_t)0x16)

/* Register Manipulation */
#define ASSIGN_REG_X_VALUEOF_REG_Y              ((uint8_t)0x1F)

/* LIPOs */
#define PUSH_DOWN_PILE_R_FROM_PIN_X             ((uint8_t)0x20)
#define PUSH_UP_PILE_R_FROM_PIN_X               ((uint8_t)0x21)

#define POP_DOWN_PILE_T_INTO_PIN_X              ((uint8_t)0x22)
#define POP_UP_PILE_T_INTO_PIN_X                ((uint8_t)0x23)

#define NEXT_NEW_INSTRUCTION                    ((uint8_t)0x25)

/* Exit */
#define EXIT                                    ((uint8_t)0xFF)

int reads_registers(uint8_t* program, uint32_t len);
int writes_registers(uint8_t* program, uint32_t len);

int reads_pile(uint8_t* program, uint32_t len);
int writes_pile(uint8_t* program, uint32_t len);

#endif

