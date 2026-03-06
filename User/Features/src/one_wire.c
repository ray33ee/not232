#include "Features/inc/one_wire.h"

static inline void one_wire_delay(uint32_t delay) {
    Delay_Us(delay);
}

uint8_t one_wire_reset(uint16_t pin_mask) {
    one_wire_delay(OW_DELAY_G);
    GPIOA->BCR = pin_mask;
    one_wire_delay(OW_DELAY_H);
    GPIOA->BSHR = pin_mask;
    one_wire_delay(OW_DELAY_I);
    uint8_t result = (GPIOA->INDR & pin_mask) == (uint32_t)Bit_RESET;
    one_wire_delay(OW_DELAY_J);

    return result;
}

void one_wire_write0(uint16_t pin_mask) {
    GPIOA->BCR = pin_mask;
    one_wire_delay(OW_DELAY_C);
    GPIOA->BSHR = pin_mask;
    one_wire_delay(OW_DELAY_D);
}

void one_wire_write1(uint16_t pin_mask) {
    GPIOA->BCR = pin_mask;
    one_wire_delay(OW_DELAY_A);
    GPIOA->BSHR = pin_mask;
    one_wire_delay(OW_DELAY_B);
}

uint8_t one_wire_read(uint16_t pin_mask) {
    GPIOA->BCR = pin_mask;
    one_wire_delay(OW_DELAY_A);
    GPIOA->BSHR = pin_mask;
    one_wire_delay(OW_DELAY_E);
    uint8_t result = (GPIOA->INDR & pin_mask) != (uint32_t)Bit_RESET;
    one_wire_delay(OW_DELAY_F);

    return result;
}