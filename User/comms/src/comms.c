#include "comms/inc/comms.h"

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

            gpio_init_adf_pins(pin_number, GPIO_Mode_Out_PP);

            break;}
        case RECV_OUT_OD:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_init_adf_pins(pin_number, GPIO_Mode_Out_OD);

            break;}
        case RECV_IN_FLOATING:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_init_adf_pins(pin_number, GPIO_Mode_IN_FLOATING);

            break;}
        case RECV_IN_PU:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_init_adf_pins(pin_number, GPIO_Mode_IPU);

            break;}
        case RECV_IN_PD:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_init_adf_pins(pin_number, GPIO_Mode_IPD);

            break;}
        case RECV_SET_PIN:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_set_adf_pin(pin_number);

            break;}
        case RECV_CLEAR_PIN:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_clear_adf_pin(pin_number);

            break;}
        case RECV_READ_PIN:
            {
            uint8_t pin_number = usbSerial_blocking_read_u8();

            uint32_t bit = gpio_read_adf_pin(pin_number);

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
            uint8_t pin_number = usbSerial_blocking_read_u8();

            gpio_init_adf_pins(pin_number, GPIO_Mode_AF_PP);
            
            break;}
        case RECV_PWM_DUTY:
            {
                
            uint8_t pin_number = usbSerial_blocking_read_u8();
            
            uint8_t duty = usbSerial_blocking_read_u8();

            pwm_set_duty(pin_number, duty);

            break;}
        case RECV_ADC_INIT:
            {
                uint8_t pin_number = usbSerial_blocking_read_u8();

                gpio_init_adf_pins(pin_number, GPIO_Mode_AIN);
 
            
            break;}
        case RECV_ADC_READ:
            {
            uint8_t pin_number = usbSerial_blocking_read_u8();

            uint32_t val = (uint32_t)adc_read(pin_number);

            usbSerial_blocking_writeP_u32(val);

            break;}
        case RECV_ADC_TKEY_READ:
            {   
            uint8_t pin_number = usbSerial_blocking_read_u8();

            uint32_t val = (uint32_t)tkey_read(pin_number);

            usbSerial_blocking_writeP_u32(val);
            break;}
        case RECV_I2C_INIT:
            {
                
            uint8_t sda_pin_number = usbSerial_blocking_read_u8();
            
            uint8_t scl_pin_number = usbSerial_blocking_read_u8();

            gpio_init_ad_pins(sda_pin_number, GPIO_Mode_Out_OD);
            gpio_init_ad_pins(scl_pin_number, GPIO_Mode_Out_OD);
            
            break;}
        case RECV_I2C_SCAN:
            {
                
            uint8_t sda_pin_number = usbSerial_blocking_read_u8();

            uint8_t scl_pin_number = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            for (int i = 1; i < 0x80; i++) {

                if (i2c_scan_address(sda_pin_number, scl_pin_number, i, delay)) {
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

            uint32_t delay = usbSerial_blocking_read_u32();

            i2c_send_bytes(sda_pin, scl_pin, address, write_buffer, write_data_len, stop, delay);

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

            uint32_t delay = usbSerial_blocking_read_u32();

            i2c_recv_bytes(sda_pin, scl_pin, address, read_buffer, bytes_to_read, stop, delay);

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

            uint32_t delay = usbSerial_blocking_read_u32();

            i2c_send_recv_bytes(sda_pin, scl_pin, address, write_buffer, write_data_len, read_buffer, bytes_to_read, stop, delay);

            // Send buffer (M bytes)
            usbSerial_blocking_writeP(read_buffer, bytes_to_read);
            
            break;}
        case RECV_PULSEIO_IN_RESUME:
            {
            uint8_t pin = usbSerial_blocking_read_u8();
            
            uint16_t trigger_duration = (uint16_t)usbSerial_blocking_read_u32();

            pulseio_in_resume(pin, trigger_duration);

            break;}
        case RECV_PULSEIO_IN_CLEAR:
            {
            
            pulseio_in_clear();

            break;}
        case RECV_PULSEIO_IN_STOP:
            {
            uint8_t pin = usbSerial_blocking_read_u8();

            pulseio_in_stop(pin);
            
            break;}
        case RECV_PULSEIO_IN_READ:
            {
            uint32_t pulse_count = usbSerial_blocking_read_u32(); //Number of pulses to read

            pulseio_in_read(pulse_count);

            break;}
        case RECV_PULSEIO_OUT_SEND:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();
            
            uint32_t duty = usbSerial_blocking_read_u32();
            
            uint32_t pulse_count = usbSerial_blocking_read_u32();

            uint16_t pulses[PIO_PULSES_SIZE];

            pulseio_out_send(pin_number, duty, pulses, pulse_count);

            break;}
        case RECV_NEOPIXEL_WRITE:
            {

            uint8_t colors[NEOPIXEL_COLOR_MAX];
            
            uint8_t pin_number = usbSerial_blocking_read_u8();

            uint32_t color_count = usbSerial_blocking_read_u32();

            for (int i = 0; i < color_count; i++) {

                colors[i] = usbSerial_blocking_read_u8();
            }

            uint16_t pin_mask = 1 << pin_number;

            neopixel_send_buffer(pin_mask, colors, color_count);
            
            break;}
        case RECV_OW_RESET:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();
            
            uint32_t result = one_wire_reset(1 << pin_number);

            usbSerial_blocking_writeP_u32(result);

            break;}
        case RECV_OW_WRITE_0:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();

            one_wire_write0(1 << pin_number);

            break;}
        case RECV_OW_WRITE_1:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();

            one_wire_write1(1 << pin_number);
            
            break;}
        case RECV_OW_READ:
            {
            
            uint8_t pin_number = usbSerial_blocking_read_u8();
            
            uint32_t result = one_wire_read(1 << pin_number);

            usbSerial_blocking_writeP_u32(result);
            
            break;}
        case RECV_SPI_INIT:
            {
            
            uint8_t clock_pin = usbSerial_blocking_read_u8();
            
            uint8_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint8_t miso_pin = usbSerial_blocking_read_u8();

            spi_init(clock_pin, mosi_pin, miso_pin);
            
            break;}
        case RECV_SPI_READ:
            {
            
            uint8_t clock_pin = usbSerial_blocking_read_u8();
            
            uint8_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint8_t miso_pin = usbSerial_blocking_read_u8();
            
            uint8_t mode = usbSerial_blocking_read_u8();
            
            uint8_t write_value = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t read_buff[1000];

            spi_read(clock_pin, mosi_pin, miso_pin, mode, write_value, delay, read_buff, len);

            usbSerial_blocking_writeP(read_buff, len);

            

            break;}
        case RECV_SPI_WRITE:
            {
            
            uint8_t clock_pin = usbSerial_blocking_read_u8();
            
            uint8_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint8_t miso_pin = usbSerial_blocking_read_u8();
            
            uint8_t mode = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t write_buff[1000];

            for (int i = 0; i < len; i++) {
                write_buff[i] = usbSerial_blocking_read_u8();
            }
            
            spi_write(clock_pin, mosi_pin, miso_pin, mode, delay, write_buff, len);
            
            break;}
        case RECV_SPI_WRITE_READ:
            {
            
            uint8_t clock_pin = usbSerial_blocking_read_u8();
            
            uint8_t mosi_pin = usbSerial_blocking_read_u8();
            
            uint8_t miso_pin = usbSerial_blocking_read_u8();
            
            uint8_t mode = usbSerial_blocking_read_u8();

            uint32_t delay = usbSerial_blocking_read_u32();

            uint32_t len = usbSerial_blocking_read_u32();

            uint8_t read_buff[1000];
            uint8_t write_buff[1000];

            for (int i = 0; i < len; i++) {
                
                write_buff[i] = usbSerial_blocking_read_u8();
            }

            
            spi_write_read(clock_pin, mosi_pin, miso_pin, mode, delay, write_buff, read_buff, len);


            usbSerial_blocking_writeP(read_buff, len);



            
            break;}
            
    }
}
