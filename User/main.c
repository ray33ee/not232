/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *USART Print debugging routine:
 *USART1_Tx(PA9).
 *This example demonstrates using USART1(PA9) as a print debug port output.
 *
 */

#include "debug.h"

#include "VMIOX/inc/execute.h"
#include "VMIOX/inc/opcodes.h"
#include "comms/inc/comms.h"
#include "comms/inc/protocol.h"
#include "Features/inc/pwm.h"
#include "Features/inc/adc.h"
#include "Features/inc/pulse_in.h"

#include "FAT/inc/lfs.h"
#include "Features/inc/fs.h"

extern lfs_t lfs;
extern lfs_file_t file;
extern const struct lfs_config cfg;
extern uint32_t current_mode;


/*

PLEASE SEE THIS LINK

https://github.com/guuuuus/ch32v20x_usbSerial

It contains an implementation of USB-CDC, read and write, which works with Moun River

*/

/* Global typedef */

/* Global define */

/* Global Variable */

/* ------------------------------------------------------------------------- */
/* littlefs block device callbacks                                            */
/* ------------------------------------------------------------------------- */

/*void test_lfs(void)
{
    uint32_t fh = flashfs_file_open("boot_count", LFS_O_RDWR | LFS_O_CREAT);

    uint32_t boot_count = 0;
    flashfs_file_read(fh, (uint8_t*)&boot_count, sizeof(boot_count));

    boot_count += 1;
    flashfs_file_seek(fh, 0, LFS_SEEK_SET);
    flashfs_file_write(fh, (uint8_t*)&boot_count, sizeof(boot_count));

    flashfs_file_close(fh);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}*/

#define SCL_MASK (1 << 8)
#define SDA_MASK (1 << 9)

#define ACK (1)
#define NACK (0)

#define STOP ((uint16_t)0xFFFF)

void i2c_device_init2() {
    //Ensure both lines start OD
    GPIOB->BSHR = SDA_MASK;
    GPIOB->BSHR = SCL_MASK;

    GPIO_InitTypeDef all;

    all.GPIO_Mode = GPIO_Mode_Out_OD;
    all.GPIO_Speed = GPIO_Speed_50MHz;

    all.GPIO_Pin = SDA_MASK | SCL_MASK;

    GPIO_Init(GPIOB, &all);
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

void i2c_device_start_condition() {
    if (i2c_device_read_scl() && i2c_device_read_sda()) {
        while (i2c_device_read_sda());
    }

    //Not strictly part of the start condition, but we check before the data
    while (i2c_device_read_scl()); //Wait for scl to come down too
}

uint16_t i2c_device_read_bit() {

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

    for (int i = 1; i < 8; i++) {
        byte <<= 1;
        byte |= i2c_device_read_bit();
    }

    i2c_device_ack(ACK, SET);

    return byte;
}

static inline uint32_t i2c_device_read2(uint8_t address, uint8_t* buffer, uint32_t size) {
    i2c_device_start_condition();

    uint16_t bus_address = i2c_device_read_byte();

    if (bus_address == STOP) {
        return -1;
    }

    if (bus_address == address << 1) {
        for (int i = 0; i < size; i++) {
            uint16_t byte = i2c_device_read_byte();


            if (byte == STOP) {
                return i;
            }

            buffer[i] = byte;
        }
        while (i2c_device_read_byte() != STOP);

        return size;
    } else {
        //Read bytes until the bus is free again
        while (i2c_device_read_byte() != STOP);
    }

    return -2;
}

static inline uint16_t i2c_device_write_bit(int bit) {

    if (bit) {
        i2c_device_set_sda();
    } else {
        i2c_device_clear_sda();
    }

    
    //Wait for SCL to go high
    while (!i2c_device_read_scl());
    //Wait for SCL to go low
    while (i2c_device_read_scl());

    
    i2c_device_clear_sda();

}

static inline uint32_t i2c_device_write2(uint8_t address, uint8_t* buffer, uint32_t size) {
    i2c_device_start_condition();

    uint16_t bus_address = i2c_device_read_byte();

    if (bus_address == STOP) {
        return -1;
    }

    if (bus_address == ((address << 1) | 1)) {
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);

        i2c_device_read_bit();

        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);
        i2c_device_write_bit(1);
        i2c_device_write_bit(0);

        i2c_device_read_bit();
    }
}



/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    // Setup all the peripheral clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // Initialise the USB 
    comms_init(COMMS_MOVE_USB, 0x02);

    gpio_init_default();
    
    // Setup PWM timers 
    pwm_init();

    // Setup ADC 
    adc_init();
    
    // Setup pin 16, B0 as UART TX and use for printf 
    gpio_init_f_pins(16, GPIO_Mode_AF_PP);


    printf("Initialising...\r\n");

    // Mount the FLASH littfs storage 
    flashfs_init();

    // Setup pulseio timer 
    pulseio_in_init();

    printf("Starting main loop.\r\n");
    
    while(1)
    {



        if (usbSerial_available() || current_mode == COMMS_MOVE_I2C) {
            get_packet();

        }

        if (current_mode == COMMS_MOVE_USB) {
            usbSerial_flush();

        }

    }
}
