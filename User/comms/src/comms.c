#include "comms/inc/comms.h"

uint16_t ADC1_ReadChannel(uint8_t adc_channel)
{
    ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_55Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    uint16_t v = ADC_GetConversionValue(ADC1);

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    return v;
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

static inline void gpio_write_fast(uint16_t mask, uint8_t level)
{
    if (level) GPIOA->BSHR = mask;
    else       GPIOA->BCR  = mask;
}

static inline uint8_t gpio_read_fast(uint16_t mask)
{
    return (GPIOA->INDR & mask) ? 1u : 0u;
}

static inline void spi_delay_nops(uint32_t n)
{
    while (n--) __NOP();
}

uint8_t spi_xfer_byte_bb_ch32(uint8_t sck_pin,
                              uint8_t mosi_pin,
                              uint8_t miso_pin,
                              uint8_t out,
                              uint8_t mode,
                              uint32_t edge_delay_nops)
{

    uint8_t cpha = mode & 1;
    uint8_t cpol = mode >> 1;

    const uint16_t sck_mask  = (uint16_t)(1u << sck_pin);
    const uint16_t mosi_mask = (uint16_t)(1u << mosi_pin);
    const uint16_t miso_mask = (uint16_t)(1u << miso_pin);

    const uint8_t idle   = (cpol != 0u) ? 1u : 0u;
    const uint8_t active = (uint8_t)(idle ^ 1u);

    uint8_t in = 0;

    // Start at idle and hold for half-cycle (helps consistent first pulse width)
    gpio_write_fast(sck_mask, idle);
    spi_delay_nops(edge_delay_nops);

    if (cpha == 0u) {
        // CPHA=0: set MOSI before leading edge, sample on leading edge
        for (uint8_t i = 0u; i < 8u; i++) {
            // Drive MOSI bit
            gpio_write_fast(mosi_mask, (out & 0x80u) ? 1u : 0u);
            out <<= 1;

            // Leading edge -> active
            gpio_write_fast(sck_mask, active);
            spi_delay_nops(edge_delay_nops);

            // Sample MISO
            in = (uint8_t)((in << 1) | gpio_read_fast(miso_mask));

            // Trailing edge -> idle
            gpio_write_fast(sck_mask, idle);
            spi_delay_nops(edge_delay_nops);
        }
    } else {
        // CPHA=1: leading edge first, sample on trailing edge
        for (uint8_t i = 0u; i < 8u; i++) {
            // Leading edge -> active
            gpio_write_fast(sck_mask, active);
            spi_delay_nops(edge_delay_nops);

            // Drive MOSI bit (changes during active phase)
            gpio_write_fast(mosi_mask, (out & 0x80u) ? 1u : 0u);
            out <<= 1;

            // Trailing edge -> idle (sample edge)
            gpio_write_fast(sck_mask, idle);
            spi_delay_nops(edge_delay_nops);

            // Sample MISO
            in = (uint8_t)((in << 1) | gpio_read_fast(miso_mask));
        }
    }

    return in;
}

void get_packet() {
    uint8_t code = usbSerial_blocking_read_u8();

    switch (code) {
        case RECV_PING:
            /*
                Ping
            */
            usbSerial_blocking_writeP_u32(SEND_OK);
            break;
        case RECV_IDENTIFY:
            /*
                Identify

                Returns a response unique to each N232 Device
            */
            {uint32_t* uids = ESIG_REGISTER_BASE;

            char s[] = "TLL_N232";

            usbSerial_blocking_writeP_u32(uids[0]);
            usbSerial_blocking_writeP_u32(uids[1]);
            usbSerial_blocking_writeP_u32(uids[2]);

            usbSerial_blocking_writeP((uint8_t*)s, 8);
            break;}
        case RECV_OUT_PP:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_Out_PP);

            break;}
        case RECV_OUT_OD:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_Out_OD);

            break;}
        case RECV_IN_FLOATING:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_IN_FLOATING);

            break;}
        case RECV_IN_PU:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_IPU);

            break;}
        case RECV_IN_PD:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_IPD);

            break;}
        case RECV_SET_PIN:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIOA->BSHR = 1 << pin_number;

            break;}
        case RECV_CLEAR_PIN:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            GPIOA->BCR = 1 << pin_number;

            break;}
        case RECV_READ_PIN:
            {
            uint8_t pin_number = usbSerial_blocking_read_u8();

            uint32_t bit = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;

            usbSerial_blocking_writeP((uint8_t*)&bit, 1);
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
            uint32_t program_size = usbSerial_blocking_read_u32();

            //Get the program
            for (int i = 0; i < program_size; i++) {
                
                uint16_t b = usbSerial_blocking_read_u8();

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
                    registers[i] = usbSerial_blocking_read_u8();
                }
            }

            // If the program contains any instructions that read from the pile_t pile, then we need to load pile_t from the host
            if (reads_pile(program, program_size)) {

                tp = usbSerial_blocking_read_u32();

                for (int i = 0; i < PILE_SIZE; i++) {
                    pile_t[i] = usbSerial_blocking_read_u8();
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
            usbSerial_blocking_writeP_u32(response_len);
            usbSerial_blocking_writeP_u32(SEND_MACHINE_STATE); //Response 

            //If the program has written to registers, send them back to host
            if (writes_registers(program, program_size)) {
                usbSerial_blocking_writeP(registers, REGISTER_COUNT);
            }

            //If the program has written to the r pile, send it back to host (along with the rp value)
            if (writes_pile(program, program_size)) {
                usbSerial_blocking_writeP_u32(rp);

                usbSerial_blocking_writeP(pile_r, PILE_SIZE);
            }

            
            break;}
        case RECV_PWM_INIT:

            {
            uint32_t pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(pin_number, GPIO_Mode_AF_PP);
            
            break;}
        case RECV_PWM_DUTY:
            {
                
            uint32_t pin_number = usbSerial_blocking_read_u8();
            
            uint32_t duty = usbSerial_blocking_read_u8();

            set_duty(pin_number, duty);

            break;}
        case RECV_ADC_INIT:
            {
                uint32_t pin_number = usbSerial_blocking_read_u8();

                GPIO_Init_Small(pin_number, GPIO_Mode_AIN);
 
            
            break;}
        case RECV_ADC_READ:
            {
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t val = (uint32_t)ADC1_ReadChannel(pin_number);

            usbSerial_blocking_writeP_u32(val);

            break;}
        case RECV_I2C_INIT:
            {
                
            uint32_t sda_pin_number = usbSerial_blocking_read_u8();
            
            uint32_t scl_pin_number = usbSerial_blocking_read_u8();

            GPIO_Init_Small(sda_pin_number, GPIO_Mode_Out_OD);
            GPIO_Init_Small(scl_pin_number, GPIO_Mode_Out_OD);
            
            break;}
        case RECV_I2C_SCAN:
            {
                
            uint32_t sda_pin_number = usbSerial_blocking_read_u8();

            uint32_t scl_pin_number = usbSerial_blocking_read_u8();

            for (int i = 1; i < 0x80; i++) {

                if (i2c_scan_address(sda_pin_number, scl_pin_number, i)) {
                    uint8_t addr = i;
                    usbSerial_blocking_writeP(&addr, 1);
                }
            }

            uint8_t terminal = 0x00;
            usbSerial_blocking_writeP(&terminal, 1);

            break;}
        case RECV_I2C_WRITE:
            {
            uint8_t write_buffer[I2C_WRITE_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            
            uint8_t sda_pin = usbSerial_blocking_read_u8();
            
            uint8_t scl_pin = usbSerial_blocking_read_u8();

            // I2C address (1 byte)
            uint8_t address = usbSerial_blocking_read_u8() << 1;
            
            // Number of bytes to write to device (4 bytes)
            uint32_t write_data_len = usbSerial_blocking_read_u32();

            // Get bytes to write (N bytes)
            for (int i = 0; i < write_data_len; i++) {
                write_buffer[i] = usbSerial_blocking_read_u8();
            }

            // Stop condition (1 byte)
            uint8_t stop = usbSerial_blocking_read_u8();

            i2c_send_bytes(sda_pin, scl_pin, address, write_buffer, write_data_len, stop);

            break;}
        case RECV_I2C_READ:
            {
            uint8_t read_buffer[I2C_READ_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            
            uint8_t sda_pin = usbSerial_blocking_read_u8();
            
            uint8_t scl_pin = usbSerial_blocking_read_u8();

            // I2C address (1 byte)
            uint8_t address = usbSerial_blocking_read_u8() << 1;

            // Get number of bytes to read (4 bytes)
            uint32_t bytes_to_read = usbSerial_blocking_read_u32();

            // Stop condition (1 byte)
            uint8_t stop = usbSerial_blocking_read_u8();

            i2c_recv_bytes(sda_pin, scl_pin, address, read_buffer, bytes_to_read, stop);

            // Send buffer (M bytes)
            usbSerial_blocking_writeP(read_buffer, bytes_to_read);

            break;}
        case RECV_I2C_WRITE_READ:
            {
            uint8_t write_buffer[I2C_WRITE_BUFFER_MAX];
            uint8_t read_buffer[I2C_READ_BUFFER_MAX];

            // SDA and SCL (2 bytes)
            
            uint8_t sda_pin = usbSerial_blocking_read_u8();
            
            uint8_t scl_pin = usbSerial_blocking_read_u8();

            // I2C address (1 byte)
            uint8_t address = usbSerial_blocking_read_u8() << 1;
            
            // Number of bytes to write to device (4 bytes)
            uint32_t write_data_len = usbSerial_blocking_read_u32();

            // Get bytes to write (N bytes)
            for (int i = 0; i < write_data_len; i++) {
                write_buffer[i] = usbSerial_blocking_read_u8();
            }

            // Get number of bytes to read (4 bytes)
            uint32_t bytes_to_read = usbSerial_blocking_read_u32();

            // Stop condition (1 byte)
            uint8_t stop = usbSerial_blocking_read_u8();

            i2c_send_recv_bytes(sda_pin, scl_pin, address, write_buffer, write_data_len, read_buffer, bytes_to_read, stop);

            // Send buffer (M bytes)
            usbSerial_blocking_writeP(read_buffer, bytes_to_read);
            
            break;}
        case RECV_PULSEIO_RESUME:
            {
            uint8_t pin = usbSerial_blocking_read_u8();
            
            uint16_t trigger_duration = (uint16_t)usbSerial_blocking_read_u32();

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
            uint8_t pin = usbSerial_blocking_read_u8();

            EXTI_DEINIT(pin);

            TIM4_pulseio_stop();
            
            break;}
        case RECV_PULSEIO_LENGTH:

            {__disable_irq();

            uint32_t pio_count_copy = pio_count;

            __enable_irq();


            usbSerial_blocking_writeP_u32(pio_count_copy);

            break;}
        case RECV_PULSEIO_POPLEFT:

            {__disable_irq();

            uint32_t pio_head_copy = pio_head;
            uint32_t pio_count_copy = pio_count;

            pio_count--;

            __enable_irq();

            uint32_t tail = (pio_head_copy + PIO_RING_SIZE - pio_count_copy) % PIO_RING_SIZE;

            uint16_t val = pio_buffer[tail];

            usbSerial_blocking_writeP_u32(val);


            break;}
        case RECV_PULSEIO_READ:
            {
            uint32_t pulse_count = usbSerial_blocking_read_u32(); //Number of pulses to read

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

            usbSerial_blocking_writeP_u32(modified_count);

            uint32_t tail = (pio_head_copy + PIO_RING_SIZE - modified_count) % PIO_RING_SIZE;

            for (int i = 0; i < modified_count; i++) {
                uint16_t value = pio_buffer[(tail + i) % PIO_RING_SIZE];
                usbSerial_blocking_writeP_u32(value);
            }

            break;}
        case RECV_PULSEIO_OUT:
            {
            
            uint32_t pin_number = usbSerial_blocking_read_u8();
            
            uint32_t duty = usbSerial_blocking_read_u32();
            
            uint32_t pulse_count = usbSerial_blocking_read_u32();

            uint16_t pulses[PIO_PULSES_SIZE];

            for (int i = 0; i < pulse_count; i++) {
                pulses[i] = (uint16_t)usbSerial_blocking_read_u32();
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
            
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t color_count = usbSerial_blocking_read_u32();

            for (int i = 0; i < color_count; i++) {

                colors[i] = usbSerial_blocking_read_u8();
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
            
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t mask = 1 << pin_number;
            
            uint32_t result;

            ow_delay(OW_DELAY_G);
            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_H);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_I);
            result = (GPIOA->INDR & mask) == (uint32_t)Bit_RESET;
            ow_delay(OW_DELAY_J);

            usbSerial_blocking_writeP_u32(result);

            break;}
        case RECV_OW_WRITE_0:
            {
            
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t mask = 1 << pin_number;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_C);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_D);

            break;}
        case RECV_OW_WRITE_1:
            {
            
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t mask = 1 << pin_number;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_A);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_B);
            
            break;}
        case RECV_OW_READ:
            {
            
            uint32_t pin_number = usbSerial_blocking_read_u8();

            uint32_t mask = 1 << pin_number;
            
            uint32_t result;

            GPIOA->BCR = mask;
            ow_delay(OW_DELAY_A);
            GPIOA->BSHR = mask;
            ow_delay(OW_DELAY_E);
            result = (GPIOA->INDR & mask) != (uint32_t)Bit_RESET;
            ow_delay(OW_DELAY_F);

            usbSerial_blocking_writeP_u32(result);
            
            break;}
        case RECV_SPI_INIT:
            {
            
            uint32_t clock_pin = usbSerial_blocking_read_u8();
            
            uint32_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint32_t miso_pin = usbSerial_blocking_read_u8();

            GPIO_Init_Small(clock_pin, GPIO_Mode_Out_PP);
            GPIO_Init_Small(mosi_pin, GPIO_Mode_Out_PP);
            GPIO_Init_Small(miso_pin, GPIO_Mode_IN_FLOATING);
            
            break;}
        case RECV_SPI_READ:
            {
            
            uint32_t clock_pin = usbSerial_blocking_read_u8();
            
            uint32_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint32_t miso_pin = usbSerial_blocking_read_u8();
            
            uint32_t mode = usbSerial_blocking_read_u8();
            
            uint32_t write_value = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t read_buff[1000];

            for (int i = 0; i < len; i++) {
                read_buff[i] = spi_xfer_byte_bb_ch32(clock_pin, mosi_pin, miso_pin, write_value, mode, delay);
            }

            usbSerial_blocking_writeP(read_buff, len);

            

            break;}
        case RECV_SPI_WRITE:
            {
            
            uint32_t clock_pin = usbSerial_blocking_read_u8();
            
            uint32_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint32_t miso_pin = usbSerial_blocking_read_u8();
            
            uint32_t mode = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t write_buff[1000];

            for (int i = 0; i < len; i++) {
                
                write_buff[i] = usbSerial_blocking_read_u8();
            }

            for (int i = 0; i < len; i++) {
                spi_xfer_byte_bb_ch32(clock_pin, mosi_pin, miso_pin, write_buff[i], mode, delay);
            }
            
            
            break;}
        case RECV_SPI_WRITE_READ:
            {
            
            uint32_t clock_pin = usbSerial_blocking_read_u8();
            
            uint32_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint32_t miso_pin = usbSerial_blocking_read_u8();
            
            uint32_t mode = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t read_buff[1000];
            uint8_t write_buff[1000];

            for (int i = 0; i < len; i++) {
                
                write_buff[i] = usbSerial_blocking_read_u8();
            }

            for (int i = 0; i < len; i++) {
                read_buff[i] = spi_xfer_byte_bb_ch32(clock_pin, mosi_pin, miso_pin, write_buff[i], mode, delay);
            }


            usbSerial_blocking_writeP(read_buff, len);



            
            break;}
            
    }
}
