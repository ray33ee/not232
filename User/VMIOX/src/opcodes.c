#include "VMIOX/inc/opcodes.h"

int reads_registers(uint8_t* program, uint32_t len) {
    for (int i = 0; i < len; i += 3) {
        if (program[i] == SHIFT_LEFT_OUT_PIN_X_FROM_REG_Y || 
            program[i] == SHIFT_RIGHT_OUT_PIN_X_FROM_REG_Y || 
            program[i] == READ_BANK_A_REG_X ||
            program[i] == READ_BANK_D_REG_X ||
            program[i] == ASSIGN_REG_X_VALUEOF_REG_Y
        ) {
            return 1;
        }
    }

    return 0;
}

int writes_registers(uint8_t* program, uint32_t len) {
    for (int i = 0; i < len; i += BYTES_PER_INSTRUCTION) {
        if (program[i] == SHIFT_LEFT_IN_PIN_X_TO_REG_Y || 
            program[i] == SHIFT_RIGHT_IN_PIN_X_TO_REG_Y || 
            program[i] == WRITE_BANK_A_REG_X ||
            program[i] == WRITE_BANK_D_REG_X ||
            program[i] == ASSIGN_REG_X_VALUEOF_REG_Y
        ) {
            return 1;
        }
    }

    return 0;
}

int reads_pile(uint8_t* program, uint32_t len) {
    for (int i = 0; i < len; i += 3) {
        if (program[i] == POP_UP_PILE_T_INTO_PIN_X || program[i] == POP_DOWN_PILE_T_INTO_PIN_X) {
            return 1;
        }
    }

    return 0;
}

int writes_pile(uint8_t* program, uint32_t len) {
    for (int i = 0; i < len; i += 3) {
        if (program[i] == PUSH_UP_PILE_R_FROM_PIN_X || program[i] == PUSH_DOWN_PILE_R_FROM_PIN_X) {
            return 1;
        }
    }

    return 0;
}