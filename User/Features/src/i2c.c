#include "Features/inc/i2c.h"

static inline void i2c_delay(uint32_t n) {
    while (n--) __NOP();
}

static inline void i2c_start(uint8_t sda_pin, uint8_t scl_pin, uint32_t delay) {
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BCR = 1 << sda_pin;
    i2c_delay(delay);
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay(delay);
}

static inline void i2c_send_bit(uint8_t sda_pin, uint8_t scl_pin, int bit, uint32_t delay) {
    if (bit) {
        GPIOA->BSHR = 1 << sda_pin;
    } else {
        GPIOA->BCR = 1 << sda_pin;
    }
    i2c_delay(delay);
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay(delay);
}

static inline uint8_t i2c_send_byte(uint8_t sda_pin, uint8_t scl_pin, uint8_t byte, uint32_t delay) {
    uint8_t ack = 1;
    for (int i = 7; i >= 0; i--) {
        i2c_send_bit(sda_pin, scl_pin, byte & (1 << i), delay);
    }

    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    ack = (GPIOA->INDR & (1 << sda_pin)) == Bit_RESET;
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << sda_pin;

    return ack;
}

static inline uint8_t i2c_recv_bit(uint8_t sda_pin, uint8_t scl_pin, uint32_t delay) {
    uint8_t bit = 0;

    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    bit = (GPIOA->INDR & (1 << sda_pin)) != Bit_RESET;
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay(delay);

    return bit;
}

static inline void i2c_recv_byte(uint8_t sda_pin, uint8_t scl_pin, uint8_t* buffer, uint8_t ack, uint32_t delay) {
    *buffer = 0;
    for (int i = 7; i >= 0; i--) {
        *buffer |= i2c_recv_bit(sda_pin, scl_pin, delay) << i;
    }

    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay(delay);
    GPIOA->BCR = (ack != 0) << sda_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BSHR = (ack != 0) << sda_pin;
}

static inline void i2c_stop(uint8_t sda_pin, uint8_t scl_pin, uint32_t delay) {
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay(delay);
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay(delay);
}

uint8_t i2c_scan_address(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint32_t delay) {
    i2c_start(sda_pin, scl_pin, delay);
    uint8_t ack = i2c_send_byte(sda_pin, scl_pin, address << 1, delay);
    i2c_stop(sda_pin, scl_pin, delay);
    
    return ack;
}

void i2c_send_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop, uint32_t delay) {
    i2c_start(sda_pin, scl_pin, delay);

    i2c_send_byte(sda_pin, scl_pin, address, delay);

    for (int i = 0; i < len; i++) {
        i2c_send_byte(sda_pin, scl_pin, buffer[i], delay);
    }
    
    if (stop) {
        i2c_stop(sda_pin, scl_pin, delay);
    }
}

void i2c_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop, uint32_t delay) {
    i2c_start(sda_pin, scl_pin, delay);

    i2c_send_byte(sda_pin, scl_pin, address | 1, delay);

    
    for (int i = 0; i < len; i++) {
        i2c_recv_byte(sda_pin, scl_pin, buffer + i, i < len-1, delay); 
    }

    if (stop) {
        i2c_stop(sda_pin, scl_pin, delay);
    }
}

void i2c_send_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* out_buffer, uint32_t out_len, uint8_t* in_buffer, uint32_t in_len, int stop, uint32_t delay) {
    i2c_start(sda_pin, scl_pin, delay);

    i2c_send_byte(sda_pin, scl_pin, address, delay);

    for (int i = 0; i < out_len; i++) {
        i2c_send_byte(sda_pin, scl_pin, out_buffer[i], delay);
    }

    i2c_start(sda_pin, scl_pin, delay);

    i2c_send_byte(sda_pin, scl_pin, address | 1, delay);

    for (int i = 0; i < in_len; i++) {
        i2c_recv_byte(sda_pin, scl_pin, in_buffer + i, i < in_len-1, delay); 
    }

    if (stop) {
        i2c_stop(sda_pin, scl_pin, delay);
    }
}
