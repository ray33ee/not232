#include "comms/inc/i2c_device.h"


#define SCL_MASK (1 << 8)
#define SDA_MASK (1 << 9)

#define ACK (1)
#define NACK (0)

#define STOP ((uint16_t)0xFFFF)

uint8_t i2c_address;

void i2c_device_init(uint8_t address) {
    //Ensure both lines start OD
    GPIOB->BSHR = SDA_MASK;
    GPIOB->BSHR = SCL_MASK;

    GPIO_InitTypeDef all;

    all.GPIO_Mode = GPIO_Mode_Out_OD;
    all.GPIO_Speed = GPIO_Speed_50MHz;

    all.GPIO_Pin = SDA_MASK | SCL_MASK;

    GPIO_Init(GPIOB, &all);

    i2c_address = address;
}

static inline uint32_t i2c_device_read_scl() {
    return (GPIOB->INDR & SCL_MASK) != (uint32_t)Bit_RESET;
}

static inline uint32_t i2c_device_read_sda() {
    return (GPIOB->INDR & SDA_MASK) != (uint32_t)Bit_RESET;
}

static inline void i2c_device_set_sda() {
    GPIOB->BSHR = SDA_MASK;
}

static inline void i2c_device_clear_sda() {
    GPIOB->BCR = SDA_MASK;
}

static inline void i2c_device_start_condition() {
    if (i2c_device_read_scl() && i2c_device_read_sda()) {
        while (i2c_device_read_sda());
    }

    //Not strictly part of the start condition, but we check before the data
    while (i2c_device_read_scl()); //Wait for scl to come down too
}

static inline void i2c_device_stop_condition() {
    
    while (1) {
        //Wait for SCL to go high
        while (!i2c_device_read_scl());

        //Read SDA
        uint16_t bit = i2c_device_read_sda();

        //If SDA changes while clock is high, this is a stop condition
        while (i2c_device_read_scl()) {
            if (i2c_device_read_sda() != bit) {
                return;
            }
        }

    }
}

static inline uint16_t i2c_device_read_bit() {

    //Wait for SCL to go high
    while (!i2c_device_read_scl());

    //Read SDA
    uint32_t bit = i2c_device_read_sda();

    //Wait for SCL to go low again
    while (i2c_device_read_scl()) {

        //If SDA changes during the High clock pulse, this is a stop condition
        if (i2c_device_read_sda() != bit) {
            return STOP;
        }
    }
    
    return bit;
}

static inline void i2c_device_ack(int ack, int sda) {

    if (ack) {
        i2c_device_clear_sda();
    } else {
        i2c_device_set_sda();
    }

    //Wait for SCL to go high
    while (!i2c_device_read_scl());
    //Wait for SCL to go low
    while (i2c_device_read_scl());

    //while (!i2c_device_read_scl());

    if (sda) {
        i2c_device_set_sda();
    } else {
        i2c_device_clear_sda();
    }
}

static inline uint16_t i2c_device_read_byte() {
    uint8_t byte = 0;

    uint32_t first_bit = i2c_device_read_bit();

    //If the first bit is a stop condition, this is the end of the transmission
    if (first_bit == STOP) {
        return STOP;
    }

    byte |= first_bit;

    for (int i = 1; i < 8; i++) {
        byte <<= 1;
        byte |= i2c_device_read_bit();
    }

    i2c_device_ack(ACK, SET);

    return byte;
}

uint32_t i2c_device_read(uint8_t* buffer, uint32_t size) {

retry:

    i2c_device_start_condition();

    uint16_t bus_address;

    bus_address = i2c_device_read_byte();

    if (bus_address == STOP) {
        goto retry;
    }

    if (bus_address == i2c_address << 1) {
        for (int i = 0; i < size; i++) {
            uint16_t byte = i2c_device_read_byte();


            if (byte == STOP) {
                i2c_device_stop_condition();
                return i;
            }

            buffer[i] = byte;
        }
        
        i2c_device_stop_condition();

        return size;
    } else {
        //Read bytes until the bus is free again
        //while (i2c_device_read_byte() != STOP);

        i2c_device_stop_condition();

        //Retry the read
        goto retry;
    }

    return -2;
}

static inline void i2c_device_write_bit(int bit) {

    if (bit) {
        i2c_device_set_sda();
    } else {
        i2c_device_clear_sda();
    }

    
    //Wait for SCL to go high
    while (!i2c_device_read_scl());
    //Wait for SCL to go low
    while (i2c_device_read_scl());

    
    //i2c_device_clear_sda();

}

static inline uint16_t i2c_device_write_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        i2c_device_write_bit(byte & 0x80);
        byte <<= 1;
    }

    i2c_device_set_sda();

    uint16_t ack = i2c_device_read_bit();

    return ack;
}

uint32_t i2c_device_write(uint8_t* buffer, uint32_t size) {

retry_2:

    i2c_device_start_condition();

    uint16_t bus_address = i2c_device_read_byte();

    if (bus_address == STOP) {
        goto retry_2;
    }

    if (bus_address == ((i2c_address << 1) | 1)) {

        for (int i = 0; i < size; i++) {
            uint8_t byte = buffer[i];

            uint8_t nack = i2c_device_write_byte(byte);

            if (nack) {
                i2c_device_stop_condition();

                return i;
            }
        }

        i2c_device_stop_condition();

        return size;


    } else {

        i2c_device_stop_condition();
        goto retry_2;
    }
    return -2;

}

