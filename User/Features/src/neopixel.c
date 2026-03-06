#include "Features/inc/neopixel.h"


static inline void neopixel_send_0(uint16_t pin_mask) {
    GPIOA->BSHR = pin_mask;
    for (int i = 0; i < 11; i++) __NOP();
    GPIOA->BCR = pin_mask;
    for (int i = 0; i < 31; i++) __NOP();
}

static inline void neopixel_send_1(uint16_t pin_mask) {
    GPIOA->BSHR = pin_mask;
    for (int i = 0; i < 24; i++) __NOP();
    GPIOA->BCR = pin_mask;
    for (int i = 0; i < 18; i++) __NOP();
}

static inline void neopixel_send_bit(uint16_t pin_mask, uint32_t bit) {
    if (bit) {
        neopixel_send_1(pin_mask);
    } else {
        neopixel_send_0(pin_mask);
    }
}

static inline void neopixel_send_byte(uint16_t pin_mask, uint8_t byte) {
    neopixel_send_bit(pin_mask, byte & 0x80);
    neopixel_send_bit(pin_mask, byte & 0x40);
    neopixel_send_bit(pin_mask, byte & 0x20);
    neopixel_send_bit(pin_mask, byte & 0x10);
    neopixel_send_bit(pin_mask, byte & 0x08);
    neopixel_send_bit(pin_mask, byte & 0x04);
    neopixel_send_bit(pin_mask, byte & 0x02);
    neopixel_send_bit(pin_mask, byte & 0x01);
}

// You might expect the params to be R, G B, but some devices use different orders so we use more generic names
static inline void neopixel_send_color(uint16_t pin_mask, uint8_t b0, uint8_t b1, uint8_t b2) {
    neopixel_send_byte(pin_mask, b0);
    neopixel_send_byte(pin_mask, b1);
    neopixel_send_byte(pin_mask, b2);
}

void neopixel_send_buffer(uint16_t pin_mask, uint8_t* buffer, uint32_t len) {
    for (int i = 0; i < len; i+=3) {
        uint8_t b0 = buffer[i];
        uint8_t b1 = buffer[i+1];
        uint8_t b2 = buffer[i+2];

        neopixel_send_color(pin_mask, b0, b1, b2);

        Delay_Us(200);
    }
}