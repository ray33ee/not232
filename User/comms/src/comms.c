#include "comms/inc/comms.h"

uint32_t get_le_u32() {
    while (!usbSerial_available());
    uint8_t b1 = usbSerial_read();
    while (!usbSerial_available());
    uint8_t b2 = usbSerial_read();
    while (!usbSerial_available());
    uint8_t b3 = usbSerial_read();
    while (!usbSerial_available());
    uint8_t b4 = usbSerial_read();
    return b1 | b2 << 8 | b3 << 16 | b4 << 24;
}

void set_le_u32(uint32_t value) {
    usbSerial_writeP((unsigned char*)&value, 4);
    usbSerial_flush();
}

uint16_t ADC1_ReadChannel(uint8_t adc_channel)
{
    ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_55Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    uint16_t v = ADC_GetConversionValue(ADC1);

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    return v;
}

static inline void i2c_delay() {
    Delay_Us(1);
}

static inline void i2c_start(uint16_t sda_pin, uint16_t scl_pin) {
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BCR = 1 << sda_pin;
    i2c_delay();
    GPIOA->BCR = 1 << scl_pin;
    i2c_delay();
}

static inline void i2c_send_bit(uint16_t sda_pin, uint16_t scl_pin, int bit) {
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

static inline uint8_t i2c_send_byte(uint16_t sda_pin, uint16_t scl_pin, uint8_t byte) {
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

static inline uint8_t i2c_recv_bit(uint16_t sda_pin, uint16_t scl_pin) {
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

static inline void i2c_recv_byte(uint16_t sda_pin, uint16_t scl_pin, uint8_t* buffer, uint8_t ack) {
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

static inline void i2c_stop(uint16_t sda_pin, uint16_t scl_pin) {
    GPIOA->BSHR = 1 << scl_pin;
    i2c_delay();
    GPIOA->BSHR = 1 << sda_pin;
    i2c_delay();
}



void EXTI_INIT(uint16_t pin)
{
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    /* Map Px<pin> -> EXTI line <pin> */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, pin);

    /* EXTI line mask: EXTI_Line0 << pin */
    uint32_t line = (uint32_t)(1u << pin);

    EXTI_InitStructure.EXTI_Line    = 1 << pin;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Clear any pending flag to avoid immediate interrupt */
    EXTI_ClearITPendingBit(line);

    /* Select NVIC IRQ for this EXTI line */
    IRQn_Type irqn;
    if (pin <= 4u) {
        irqn = (IRQn_Type)(EXTI0_IRQn + pin);   /* EXTI0..EXTI4 */
    } else if (pin <= 9u) {
        irqn = EXTI9_5_IRQn;                    /* EXTI5..EXTI9 shared */
    } else {
        irqn = EXTI15_10_IRQn;                  /* EXTI10..EXTI15 shared */
    }

    NVIC_InitStructure.NVIC_IRQChannel                   = irqn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * Disable EXTI interrupt on Px<pin> (pin 0..15)
 */
void EXTI_DEINIT(uint8_t pin)
{
    uint32_t line = (uint32_t)(1u << pin);

    /* Disable EXTI line */
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    EXTI_InitStructure.EXTI_Line    = line;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Clear pending bit */
    EXTI_ClearITPendingBit(line);

}

void set_duty(uint16_t pin_number, uint16_t duty) {
    if (pin_number == 0) {
        TIM2->CH1CVR = duty;
    } else if (pin_number == 1) {
        TIM2->CH2CVR = duty;
    } else if (pin_number == 2) {
        TIM2->CH3CVR = duty;
    } else if (pin_number == 3) {
        TIM2->CH4CVR = duty;
    } else if (pin_number == 6) {
        TIM3->CH1CVR = duty;
    } else if (pin_number == 7) {
        TIM3->CH2CVR = duty;
    } else if (pin_number == 8) {
        TIM1->CH1CVR = duty;
    } else if (pin_number == 9) {
        TIM1->CH2CVR = duty;
    } else if (pin_number == 10) {
        TIM1->CH3CVR = duty;
    } else if (pin_number == 11) {
        TIM1->CH4CVR = duty;
    }
}

void neopixel_send_0(uint16_t pin_mask) {
    GPIOA->BSHR = pin_mask;
    for (int i = 0; i < 11; i++) __NOP();
    GPIOA->BCR = pin_mask;
    for (int i = 0; i < 31; i++) __NOP();
}

void neopixel_send_1(uint16_t pin_mask) {
    GPIOA->BSHR = pin_mask;
    for (int i = 0; i < 24; i++) __NOP();
    GPIOA->BCR = pin_mask;
    for (int i = 0; i < 18; i++) __NOP();
}

void neopixel_send_bit(uint16_t pin_mask, uint32_t bit) {
    if (bit) {
        neopixel_send_1(pin_mask);
    } else {
        neopixel_send_0(pin_mask);
    }
}

void neopixel_send_byte(uint16_t pin_mask, uint8_t byte) {
    neopixel_send_bit(pin_mask, byte & 0x80);
    neopixel_send_bit(pin_mask, byte & 0x40);
    neopixel_send_bit(pin_mask, byte & 0x20);
    neopixel_send_bit(pin_mask, byte & 0x10);
    neopixel_send_bit(pin_mask, byte & 0x08);
    neopixel_send_bit(pin_mask, byte & 0x04);
    neopixel_send_bit(pin_mask, byte & 0x02);
    neopixel_send_bit(pin_mask, byte & 0x01);
}

void ow_delay(uint32_t delay) {
    Delay_Us(delay);
}

void get_packet() {
    while (!usbSerial_available());
    uint8_t code = usbSerial_read();

    switch (code) {
        case RECV_PING:
            /*
                Ping
            */
            set_le_u32(SEND_OK);
            break;
        case RECV_IDENTIFY:
            /*
                Identify

                Returns a response unique to each N232 Device
            */
            {uint32_t* uids = ESIG_REGISTER_BASE;

            char s[] = "TLL_N232";

            set_le_u32(uids[0]);
            set_le_u32(uids[1]);
            set_le_u32(uids[2]);

            //usbSerial_writeP((uint8_t*)ESIG_REGISTER_BASE, 12);

            usbSerial_writeP((uint8_t*)s, 8);
            usbSerial_flush();
            break;}
        case RECV_OUT_PP:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_Out_PP);

            break;}
        case RECV_OUT_OD:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_Out_OD);

            break;}
        case RECV_IN_FLOATING:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_IN_FLOATING);

            break;}
        case RECV_IN_PU:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_IPU);

            break;}
        case RECV_IN_PD:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_IPD);

            break;}
        case RECV_SET_PIN:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIOA->BSHR = 1 << pin_number;

            break;}
        case RECV_CLEAR_PIN:
            {
                
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            GPIOA->BCR = 1 << pin_number;

            break;}
        case RECV_READ_PIN:
            {
            while (!usbSerial_available());
            uint8_t pin_number = usbSerial_read();

            uint32_t bit = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;

            usbSerial_writeP((uint8_t*)&bit, 1);
            usbSerial_flush();
            break;}
            
        case RECV_RUN:
            {
            uint8_t registers[REGISTER_COUNT];

            uint8_t pile_t[PILE_SIZE]; //Transmit pile - a stack of bits that are sent out
            uint8_t pile_r[PILE_SIZE]; //Receive pile - a stack of bits received

            uint8_t program[MAX_PROGRAM_SIZE_BYTES];

            uint32_t tp = 0;

            uint32_t rp = 0;
            
            //Get the first u32 which contains the program size
            uint32_t program_size = get_le_u32();

            //Get the program
            for (int i = 0; i < program_size; i++) {
                
                while (!usbSerial_available());
                uint16_t b = usbSerial_read();

                program[i] = b;
            }

            //If the program writes to the registers, clear them
            if (writes_registers(program, program_size)) {
                for (int i = 0; i < REGISTER_COUNT; i++) {
                    registers[i] = 0;
                }
            }

            // If the program contains any instructions that read from the registers, then we need to load the registers from the host
            if (reads_registers(program, program_size)) {
                //printf("Read regs\r\n");
                for (int i = 0; i < REGISTER_COUNT; i++) {
                    while (!usbSerial_available());
                    registers[i] = usbSerial_read();
                }
            }

            // If the program contains any instructions that read from the pile_t pile, then we need to load pile_t from the host
            if (reads_pile(program, program_size)) {

                tp = get_le_u32();

                for (int i = 0; i < PILE_SIZE; i++) {
                    while (!usbSerial_available());
                    pile_t[i] = usbSerial_read();
                }
            }

            //If the program writes to the r pile, clear it
            if (writes_pile(program, program_size)) {
                for (int i = 0; i < PILE_SIZE; i++) {
                    pile_r[i] = 0;
                }
            }

            // Run program
            run(program, registers, program_size, pile_t, pile_r, &tp, &rp);

            //Based on the instructions used, compute the length of the responst
            uint32_t response_len = 0;

            if (writes_registers(program, program_size)) {
                response_len += REGISTER_COUNT;
            }

            if (writes_pile(program, program_size)) {
                response_len += PILE_SIZE + 4;
            }

            // Send response
            set_le_u32(response_len);
            set_le_u32(SEND_MACHINE_STATE); //Response 

            //If the program has written to registers, send them back to host
            if (writes_registers(program, program_size)) {
                usbSerial_writeP(registers, REGISTER_COUNT);
                usbSerial_flush();
            }

            //If the program has written to the r pile, send it back to host (along with the rp value)
            if (writes_pile(program, program_size)) {
                set_le_u32(rp);

                usbSerial_writeP(pile_r, PILE_SIZE);
                usbSerial_flush();
            }

            
            break;}
        case RECV_PWM_INIT:

            {
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            GPIO_Init_Small(pin_number, GPIO_Mode_AF_PP);
            
            break;}
        case RECV_PWM_DUTY:
            {
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();
            while (!usbSerial_available());
            uint32_t duty = usbSerial_read();

            set_duty(pin_number, duty);

            break;}
        case RECV_ADC_INIT:
            {
                while (!usbSerial_available());
                uint32_t pin_number = usbSerial_read();

                GPIO_Init_Small(pin_number, GPIO_Mode_AIN);
 
            
            break;}
        case RECV_ADC_READ:
            {
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t val = (uint32_t)ADC1_ReadChannel(pin_number);

            set_le_u32(val);

            break;}
        case RECV_I2C_INIT:
            {
            while (!usbSerial_available());
            uint32_t sda_pin_number = usbSerial_read();
            while (!usbSerial_available());
            uint32_t scl_pin_number = usbSerial_read();

            GPIO_Init_Small(sda_pin_number, GPIO_Mode_Out_OD);
            GPIO_Init_Small(scl_pin_number, GPIO_Mode_Out_OD);
            
            break;}
        case RECV_I2C_SCAN:
            {
            while (!usbSerial_available());
            uint32_t sda_pin_number = usbSerial_read();
            while (!usbSerial_available());
            uint32_t scl_pin_number = usbSerial_read();

            for (int i = 1; i < 0x80; i++) {

                i2c_start(sda_pin_number, scl_pin_number);
                uint8_t ack = i2c_send_byte(sda_pin_number, scl_pin_number, i << 1);
                i2c_stop(sda_pin_number, scl_pin_number);
                
                if (ack) {
                    uint8_t addr = i;
                    usbSerial_writeP(&addr, 1);
                }
            }

            uint8_t terminal = 0x00;
            usbSerial_writeP(&terminal, 1);
            usbSerial_flush();

            break;}
        case RECV_I2C_WRITE:
            {
            uint8_t write_buffer[I2C_WRITE_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            while (!usbSerial_available());
            uint8_t sda_pin = usbSerial_read();
            while (!usbSerial_available());
            uint8_t scl_pin = usbSerial_read();

            // I2C address (1 byte)
            while (!usbSerial_available());
            uint8_t address = usbSerial_read() << 1;
            
            // Number of bytes to write to device (4 bytes)
            uint32_t write_data_len = get_le_u32();

            // Get bytes to write (N bytes)
            for (int i = 0; i < write_data_len; i++) {
                while (!usbSerial_available());
                write_buffer[i] = usbSerial_read();
            }

            // Stop condition (1 byte)
            while (!usbSerial_available());
            uint8_t stop = usbSerial_read();

            i2c_start(sda_pin, scl_pin);

            i2c_send_byte(sda_pin, scl_pin, address);

            for (int i = 0; i < write_data_len; i++) {
                i2c_send_byte(sda_pin, scl_pin, write_buffer[i]);
            }

            
            if (stop) {
                i2c_stop(sda_pin, scl_pin);
            }




            
            break;}
        case RECV_I2C_READ:
            {
            uint8_t read_buffer[I2C_READ_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            while (!usbSerial_available());
            uint8_t sda_pin = usbSerial_read();
            while (!usbSerial_available());
            uint8_t scl_pin = usbSerial_read();

            // I2C address (1 byte)
            while (!usbSerial_available());
            uint8_t address = usbSerial_read() << 1;

            // Get number of bytes to read (4 bytes)
            uint32_t bytes_to_read = get_le_u32();

            // Stop condition (1 byte)
            while (!usbSerial_available());
            uint8_t stop = usbSerial_read();

            i2c_start(sda_pin, scl_pin);

            i2c_send_byte(sda_pin, scl_pin, address | 1);

            
            for (int i = 0; i < bytes_to_read; i++) {
                i2c_recv_byte(sda_pin, scl_pin, read_buffer + i, i < bytes_to_read-1); 
            }

            if (stop) {
                i2c_stop(sda_pin, scl_pin);
            }

            // Send buffer (M bytes)
            usbSerial_writeP(read_buffer, bytes_to_read);
            usbSerial_flush();

            break;}
        case RECV_I2C_WRITE_READ:
            {
            uint8_t write_buffer[I2C_WRITE_BUFFER_MAX];
            uint8_t read_buffer[I2C_READ_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            while (!usbSerial_available());
            uint8_t sda_pin = usbSerial_read();
            while (!usbSerial_available());
            uint8_t scl_pin = usbSerial_read();

            // I2C address (1 byte)
            while (!usbSerial_available());
            uint8_t address = usbSerial_read() << 1;
            
            // Number of bytes to write to device (4 bytes)
            uint32_t write_data_len = get_le_u32();

            // Get bytes to write (N bytes)
            for (int i = 0; i < write_data_len; i++) {
                while (!usbSerial_available());
                write_buffer[i] = usbSerial_read();
            }

            // Get number of bytes to read (4 bytes)
            uint32_t bytes_to_read = get_le_u32();

            // Stop condition (1 byte)
            while (!usbSerial_available());
            uint8_t stop = usbSerial_read();

            //Start I2C transaction
            i2c_start(sda_pin, scl_pin);

            i2c_send_byte(sda_pin, scl_pin, address);

            for (int i = 0; i < write_data_len; i++) {
                i2c_send_byte(sda_pin, scl_pin, write_buffer[i]);
            }

            i2c_start(sda_pin, scl_pin);

            i2c_send_byte(sda_pin, scl_pin, address | 1);

            for (int i = 0; i < bytes_to_read; i++) {
                i2c_recv_byte(sda_pin, scl_pin, read_buffer + i, i < bytes_to_read-1); 
            }

            if (stop) {
                i2c_stop(sda_pin, scl_pin);
            }

            // Send buffer (M bytes)
            usbSerial_writeP(read_buffer, bytes_to_read);
            usbSerial_flush();
            
            break;}
        case RECV_PULSEIO_RESUME:
            {
            while (!usbSerial_available());
            uint8_t pin = usbSerial_read();
            
            uint16_t trigger_duration = (uint16_t)get_le_u32();

            if (trigger_duration != 0) {

                GPIO_Init_Small(pin, GPIO_Mode_Out_OD);

                GPIOA->BCR = 1 << pin;

                Delay_Us(trigger_duration);
                
                GPIOA->BSHR = 1 << pin;
            }


            GPIO_Init_Small(pin, GPIO_Mode_IN_FLOATING);

            //Setup the EXTI on the pin
            EXTI_INIT(pin);

            //Start timer
            TIM4_pulseio_start();

            break;}
        case RECV_PULSEIO_CLEAR:
            {
            
            __disable_irq();
            pio_head = 0;
            pio_count = 0;
            __enable_irq();

            break;}
        case RECV_PULSEIO_STOP:
            {
            while (!usbSerial_available());
            uint8_t pin = usbSerial_read();

            EXTI_DEINIT(pin);

            TIM4_pulseio_stop();
            
            break;}
        case RECV_PULSEIO_LENGTH:

            {__disable_irq();

            uint32_t pio_count_copy = pio_count;

            __enable_irq();


            set_le_u32(pio_count_copy);

            break;}
        case RECV_PULSEIO_POPLEFT:

            {__disable_irq();

            uint32_t pio_head_copy = pio_head;
            uint32_t pio_count_copy = pio_count;

            pio_count--;

            __enable_irq();

            uint32_t tail = (pio_head_copy + PIO_RING_SIZE - pio_count_copy) % PIO_RING_SIZE;

            uint16_t val = pio_buffer[tail];

            set_le_u32(val);


            break;}
        case RECV_PULSEIO_READ:
            {
            uint32_t pulse_count = get_le_u32(); //Number of pulses to read

            __disable_irq();

            uint32_t pio_head_copy = pio_head;
            uint32_t pio_count_copy = pio_count;

            __enable_irq();
            
            uint32_t modified_count;

            if (pio_count_copy < pulse_count) {
                modified_count = pio_count_copy;
            } else {
                modified_count = pulse_count;
            }

            printf("actual: %i, pulse: %i\r\n", pio_count_copy, pulse_count);
            set_le_u32(modified_count);

            printf("actual: %i\r\n", modified_count);

            uint32_t tail = (pio_head_copy + PIO_RING_SIZE - modified_count) % PIO_RING_SIZE;

            for (int i = 0; i < modified_count; i++) {
                uint16_t value = pio_buffer[(tail + i) % PIO_RING_SIZE];
                set_le_u32(value);
            }

            break;}
        case RECV_PULSEIO_OUT:
            {
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();
            
            uint32_t duty = get_le_u32();
            
            uint32_t pulse_count = get_le_u32();

            uint16_t pulses[PIO_PULSES_SIZE];

            for (int i = 0; i < pulse_count; i++) {
                pulses[i] = (uint16_t)get_le_u32();
            }

            for (int i = 0; i < pulse_count; i++) {
                if (i & 1) {
                    //Odd index => duty = 0
                    set_duty(pin_number, 0);
                } else {
                    //Even index => duty = duty
                    set_duty(pin_number, duty);
                }
                Delay_Us(pulses[i]);
            }

            set_duty(pin_number, 0);

            break;}
        case RECV_NEOPIXEL_WRITE:
            {

            uint8_t colors[NEOPIXEL_COLOR_MAX];
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t color_count = get_le_u32();

            for (int i = 0; i < color_count; i++) {
                while (!usbSerial_available());

                colors[i] = usbSerial_read();
            }

            uint16_t pin_mask = 1 << pin_number;

            for (int i = 0; i < color_count; i+=3) {
                uint8_t b0 = colors[i];
                uint8_t b1 = colors[i+1];
                uint8_t b2 = colors[i+2];

                neopixel_send_byte(pin_mask, b0);
                neopixel_send_byte(pin_mask, b1);
                neopixel_send_byte(pin_mask, b2);

                Delay_Us(200);
            }
            
            break;}
        case RECV_OW_RESET:
            {
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t mask = 1 << pin_number;
            
            uint32_t result;

            ow_delay(OW_DELAY_G);
            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_H);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_I);
            result = (GPIOA->INDR & mask) == (uint32_t)Bit_RESET;
            ow_delay(OW_DELAY_J);

            set_le_u32(result);

            break;}
        case RECV_OW_WRITE_0:
            {
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t mask = 1 << pin_number;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_C);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_D);

            break;}
        case RECV_OW_WRITE_1:
            {
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t mask = 1 << pin_number;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_A);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_B);
            
            break;}
        case RECV_OW_READ:
            {
            
            while (!usbSerial_available());
            uint32_t pin_number = usbSerial_read();

            uint32_t mask = 1 << pin_number;
            
            uint32_t result;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_A);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_E);
            result = (GPIOA->INDR & mask) != (uint32_t)Bit_RESET;
            ow_delay(OW_DELAY_F);

            set_le_u32(result);
            
            break;}
        case RECV_SPI_INIT:
            {
            
            while (!usbSerial_available());
            uint32_t clock_pin = usbSerial_read();
            
            while (!usbSerial_available());
            uint32_t mosi_pin = usbSerial_read();
            
            while (!usbSerial_available());
            uint32_t miso_pin = usbSerial_read();

            
            break;}
        case RECV_SPI_READ:
            {
            
            break;}
        case RECV_SPI_WRITE:
            {
            
            break;}
        case RECV_SPI_WRITE_READ:
            {
            
            break;}
            
    }
}
