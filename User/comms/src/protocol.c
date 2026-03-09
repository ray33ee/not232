#include "comms/inc/protocol.h"

uint32_t current_mode;

#define I2C_TX_BUF_SIZE 512
static uint8_t i2c_tx_buf[I2C_TX_BUF_SIZE];
static uint32_t i2c_tx_len;

int comms_is_i2c_mode(void) {
    return (current_mode == COMMS_MOVE_I2C);
}

void comms_init(uint32_t mode, uint8_t i2c_address) {
    current_mode = mode;
    if (current_mode == COMMS_MOVE_USB) {
        USBFS_RCC_Init();
        usbSerial_begin();

        while(!usbSerial_connected()) { }
    } else if (current_mode == COMMS_MOVE_I2C) {
        i2c_device_init(i2c_address);
        i2c_tx_len = 0;
    }
}

void comms_flush_i2c(void) {
    if (i2c_tx_len > 0) {
        i2c_device_write(i2c_tx_buf, i2c_tx_len);
        i2c_tx_len = 0;
    }
}

void comms_send_buffer(uint8_t* buffer, uint32_t size) {
    if (current_mode == COMMS_MOVE_USB) {
        usbSerial_blocking_writeP(buffer, size);
    } else if (current_mode == COMMS_MOVE_I2C) {
        if (i2c_tx_len + size <= I2C_TX_BUF_SIZE) {
            for (uint32_t i = 0; i < size; i++) {
                i2c_tx_buf[i2c_tx_len++] = buffer[i];
            }
        }
    }
}

void comms_send_u8(uint8_t byte) {
    comms_send_buffer(&byte, 1);
}

void comms_send_u32(uint32_t byte) {
    comms_send_buffer((uint8_t*)&byte, 4);
}

void comms_send_array(uint8_t* array, uint32_t size) {
    comms_send_u32(size);
    comms_send_buffer(array, size);
}

void comms_send_string(char* str) {
    uint32_t str_len = strlen(str);

    comms_send_array((uint8_t*)str, str_len);
}

void comms_recv_buffer(uint8_t* buffer, uint32_t size) {
    if (current_mode == COMMS_MOVE_USB) {
        usbSerial_blocking_read_bytes(size, buffer);
    } else if (current_mode == COMMS_MOVE_I2C) {
        i2c_device_read(buffer, size);
    }
}

uint8_t comms_recv_u8() {
    uint8_t byte;
    comms_recv_buffer(&byte, 1);
    return byte;
}

uint32_t comms_recv_u32() {
    uint32_t word;
    comms_recv_buffer((uint8_t*)&word, 4);
    return word;
}

void comms_recv_array(uint8_t* array, uint32_t* size) {
    *size = comms_recv_u32();
    comms_recv_buffer(array, *size);
}

void comms_recv_string(uint8_t* str) {
    uint32_t size;
    comms_recv_array((uint8_t*)str, &size);
    str[size] = '\0';
}

