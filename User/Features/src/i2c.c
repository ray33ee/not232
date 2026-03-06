#include "Features/inc/i2c.h"


static inline void i2c_delay() {
    Delay_Us(1);
}

static inline void i2c_start(uint8_t sda_pin, uint8_t scl_pin) {
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BCR = 1 << sda_pin;
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();
}

static inline void i2c_send_bit(uint8_t sda_pin, uint8_t scl_pin, int bit) {
    if (bit) {
        GPIOA->BSHR = 1 << sda_pin;
    } else {
        GPIOA->BCR = 1 << sda_pin;
    }
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();
}

static inline uint8_t i2c_send_byte(uint8_t sda_pin, uint8_t scl_pin, uint8_t byte) {
    uint8_t ack = 1;
    for (int i = 7; i >= 0; i--) {
        i2c_send_bit(sda_pin, scl_pin, byte & (1 << i));
    }

    GPIOA->BSHR = 1 << sda_pin; //Release the SDA line
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin; //Clock the SCL
    i2c_delay();
    ack = (GPIOA->INDR & (1 << sda_pin)) == Bit_RESET; //Read the SDA line for the 9th bit - ACK
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << sda_pin; //Pull SDA back up
    i2c_delay();

    return ack;
}

static inline uint8_t i2c_recv_bit(uint8_t sda_pin, uint8_t scl_pin) {
    uint8_t bit = 0;
    
    
    GPIOA->BSHR = 1 << sda_pin; //Release the SDA line
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    bit = (GPIOA->INDR & (1 << sda_pin)) != Bit_RESET;
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();

    return bit;

}

static inline void i2c_recv_byte(uint8_t sda_pin, uint8_t scl_pin, uint8_t* buffer, uint8_t ack) {
    *buffer = 0;
    for (int i = 7; i >= 0; i--) {
        *buffer |= i2c_recv_bit(sda_pin, scl_pin) << i;
    }

    //Clock the 9th bit
    GPIOA->BSHR = 1 << sda_pin; //Release the SDA line
    i2c_delay();
    GPIOA->BCR = (ack != 0) << sda_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BSHR = (ack != 0) << sda_pin;
    i2c_delay();
}

static inline void i2c_stop(uint8_t sda_pin, uint8_t scl_pin) {
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay();
}

uint8_t i2c_scan_address(uint8_t sda_pin, uint8_t scl_pin, uint8_t address) {
    i2c_start(sda_pin, scl_pin);
    uint8_t ack = i2c_send_byte(sda_pin, scl_pin, address << 1);
    i2c_stop(sda_pin, scl_pin);
    
    return ack;
}

void i2c_send_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop) {
    i2c_start(sda_pin, scl_pin);

    i2c_send_byte(sda_pin, scl_pin, address);

    for (int i = 0; i < len; i++) {
        i2c_send_byte(sda_pin, scl_pin, buffer[i]);
    }
    
    if (stop) {
        i2c_stop(sda_pin, scl_pin);
    }
}

void i2c_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* buffer, uint32_t len, int stop) {
    i2c_start(sda_pin, scl_pin);

    i2c_send_byte(sda_pin, scl_pin, address | 1);

    
    for (int i = 0; i < len; i++) {
        i2c_recv_byte(sda_pin, scl_pin, buffer + i, i < len-1); 
    }

    if (stop) {
        i2c_stop(sda_pin, scl_pin);
    }
}

void i2c_send_recv_bytes(uint8_t sda_pin, uint8_t scl_pin, uint8_t address, uint8_t* out_buffer, uint32_t out_len, uint8_t* in_buffer, uint32_t in_len, int stop) {
    i2c_start(sda_pin, scl_pin);

    i2c_send_byte(sda_pin, scl_pin, address);

    for (int i = 0; i < out_len; i++) {
        i2c_send_byte(sda_pin, scl_pin, out_buffer[i]);
    }

    i2c_start(sda_pin, scl_pin);

    i2c_send_byte(sda_pin, scl_pin, address | 1);

    for (int i = 0; i < in_len; i++) {
        i2c_recv_byte(sda_pin, scl_pin, in_buffer + i, i < in_len-1); 
    }

    if (stop) {
        i2c_stop(sda_pin, scl_pin);
    }
}
