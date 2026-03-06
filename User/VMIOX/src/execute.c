
#include "VMIOX/inc/execute.h"

void run(uint8_t* opcodes, uint8_t* registers, uint32_t len, uint8_t* pile_t, uint8_t* pile_r, uint32_t* tp, uint32_t* rp) {

    /*
        PC

        Program counter. Address of the next instruction to execute.
    */
    uint32_t pc = 0;

    /*
        SP

        Stack pointer. LIFO stack pointer, points to the next item to add
    */
    uint32_t sp = 0;

    /* 
        TP

        Pile t pointer. Store the number of bits added to the pile. Pile T stands for transmit, and is used to send data out.

        The actual pointer can be calculated based on whether the pile is accessed up or down.
    */
    //uint32_t tp = 0;
    
    /* 
        TP

        Pile t pointer. Store the number of bits added to the pile. Pile T stands for receive, and is used to input data.

        The actual pointer can be calculated based on whether the pile is accessed up or down.
    */
    //uint32_t rp = 0;

    /*
        Stack

        LIFO call stack. Contains return addresses pushed by calls and popped by returns.
    */
    uint32_t stack[STACK_SIZE];

    while (1) {

        /*if (pc >= len) {
            return;
        }*/

        /*
            Instructions

            Each instruction is made of three bytes. 

            This is a command and two operand u8s, or a command and a single operand u16.

            Notation: X denotes either a) the first u8 if there are two operands or b) a u16 if there is one. Y Always denotes the second u8.
        
        */


        switch (opcodes[pc]) {
            /* Misc */
            case NOP:
                /*
                    Nop

                    Does not alter the machine state in any way, other than to advance the PC to the next instruction
                */
                {pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Setup Pins */
            case INPUT_FLOATING:
                /*
                    Set pin X to Floating Input

                    Sets up pin X as a GPIO_Mode_IN_FLOATING
                */
                {uint8_t pin_number = opcodes[pc+2];
                gpio_init_ad_pins(pin_number, GPIO_Mode_IN_FLOATING);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case INPUT_PULL_DOWN:
                /*
                    Set pin X to Input with internal pull down enabled

                    Sets up pin X as a GPIO_Mode_IPD
                */
                {uint8_t pin_number = opcodes[pc+2];
                gpio_init_ad_pins(pin_number, GPIO_Mode_IPD);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case INPUT_PULL_UP:
                /*
                    Set pin X to Input with internal pull up enabled

                    Sets up pin X as a GPIO_Mode_IPU
                */
                {uint8_t pin_number = opcodes[pc+2];
                gpio_init_ad_pins(pin_number, GPIO_Mode_IPU);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case OUTPUT_PP:
                /*
                    Set pin X to Output, push-pull

                    Sets up pin X as a GPIO_Mode_Out_PP
                */
                {uint8_t pin_number = opcodes[pc+2];
                gpio_init_ad_pins(pin_number, GPIO_Mode_Out_PP);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case OUTPUT_OD:
                /*
                    Set pin X to Output, open drain

                    Sets up pin X as a GPIO_Mode_Out_OD
                */
                {uint8_t pin_number = opcodes[pc+2];
                gpio_init_ad_pins(pin_number, GPIO_Mode_Out_OD);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Set/Clear */
            case SET_PIN_X:
                /*
                    Set pin X

                    Sets the desired pin, outputs 1 if PP mode or open if in OD mode
                */
                {
                uint16_t pin_mask = *(uint16_t*)(opcodes + pc + 2);
                GPIOA->BSHR = pin_mask;
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case CLEAR_PIN_X:
                /*
                    Clear pin X

                    Clears the desired pin
                */
                {
                uint16_t pin_mask = *(uint16_t*)(opcodes + pc + 2);
                GPIOA->BCR = pin_mask;
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Blocking functions */
            case WAIT_FOR_RISING_EDGE_X_TIMEOUT_Y:
                /*
                    Wait for rising edge

                    Holds the VM until either a) rising edge on pin X or b) timeout Y expires

                    The timeout is in multiples of 20 microseconds, which allows of a timeout up to 5ms
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t timeout = opcodes[pc+3];
                start_timer_us_u8(timeout);
                while ((GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET && !has_timer_timeout());
                while ((GPIOA->INDR & (1 << pin_number)) == (uint32_t)Bit_RESET && !has_timer_timeout());
                reset_timer();
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case WAIT_FOR_FALLING_EDGE_X_TIMEOUT_Y:
                /*
                    Wait for falling edge

                    Holds the VM until either a) falling edge on pin X or b) timeout Y expires

                    The timeout is in multiples of 20 microseconds, which allows of a timeout up to 5ms
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t timeout = opcodes[pc+3];
                start_timer_us_u8(timeout);
                while ((GPIOA->INDR & (1 << pin_number)) == (uint32_t)Bit_RESET && !has_timer_timeout());
                while ((GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET && !has_timer_timeout());
                reset_timer();
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case WAIT_FOR_LOW_PIN_X_TIMEOUT_Y:
                /*
                    Wait for low

                    Holds the VM until either a) low signal on pin X or b) timeout Y expires

                    The timeout is in multiples of 20 microseconds, which allows of a timeout up to 5ms
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t timeout = opcodes[pc+3];
                start_timer_us_u8(timeout);
                while ((GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET && !has_timer_timeout());
                reset_timer();
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case WAIT_FOR_HIGH_PIN_X_TIMEOUT_Y:
                /*
                    Wait for high

                    Holds the VM until either a) High signal on pin X or b) timeout Y expires

                    The timeout is in multiples of 20 microseconds, which allows of a timeout up to 5ms
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t timeout = opcodes[pc+3];
                start_timer_us_u8(timeout);
                while ((GPIOA->INDR & (1 << pin_number)) == (uint32_t)Bit_RESET && !has_timer_timeout());
                reset_timer();
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Shift functions */
            case SHIFT_LEFT_IN_PIN_X_TO_REG_Y:
                /* 
                    Shift left input pin X to register Y

                    Shifts register Y left by 1, reads the bit at pin X, insert this bit into the newly empty bit 0 of the register.

                    Please note: Reading happens with the INDR, this reads the actual value of the pin so it can be used in output mode too.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t reg = opcodes[pc+3];
                uint32_t bit = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;
                registers[reg] <<= 1;
                registers[reg] |= bit;
                pc+=BYTES_PER_INSTRUCTION;
                
                break;}
            case SHIFT_RIGHT_OUT_PIN_X_FROM_REG_Y:
                /* 
                    Shift right output pin X from register Y

                    Read bit 0 in register Y, shift the register right by 1, output the read value

                    Please note: Reading happens with the INDR, this reads the actual value of the pin so it can be used in output mode too.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t reg = opcodes[pc+3];
                uint32_t value = registers[reg] & 1;
                registers[reg] >>= 1;
                if (value) {
                    GPIOA->BSHR = 1 << pin_number;
                } else {
                    GPIOA->BCR = 1 << pin_number;
                }
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case SHIFT_RIGHT_IN_PIN_X_TO_REG_Y:
                /* 
                    Shift right input pin X to register Y

                    Shifts register Y right by 1, reads the bit at pin X, insert this bit into the newly empty bit 7 of the register.

                    Please note: Reading happens with the INDR, this reads the actual value of the pin so it can be used in output mode too.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t reg = opcodes[pc+3];
                uint32_t bit = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;
                registers[reg] >>= 1;
                registers[reg] |= bit << 7;
                
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case SHIFT_LEFT_OUT_PIN_X_FROM_REG_Y:
                /* 
                    Shift left output pin X from register Y

                    Read bit 7 in register Y, shift the register left by 1, output the read value

                    Please note: Reading happens with the INDR, this reads the actual value of the pin so it can be used in output mode too.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint8_t reg = opcodes[pc+3];
                uint32_t value = registers[reg] & 0x80;
                registers[reg] <<= 1;
                if (value) {
                    GPIOA->BSHR = 1 << pin_number;
                } else {
                    GPIOA->BCR = 1 << pin_number;
                }
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Port wide functions */
            case WRITE_BANK_A_REG_X:
                /*
                    Write A bank

                    Take the value in register X and write it to the A bank of pins (PA0-PA7).
                
                */
                {uint8_t reg = opcodes[pc+2];
                GPIOA->OUTDR = (GPIOA->OUTDR & 0xFFFFFFF0) | (registers[reg] & 0xFF);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case WRITE_BANK_D_REG_X:
                /*
                    Write D bank

                    Take the value in register X and write it to the d bank of pins (PA8-PA15).
                
                */
                {uint8_t reg = opcodes[pc+2];
                GPIOA->OUTDR = (GPIOA->OUTDR & 0xFFFFFF0F) | ((registers[reg] & 0xFF) << 8);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case READ_BANK_A_REG_X:
                /*
                    Read A bank

                    Read the A bank (PA0-PA7), insert the byte into register X
                */
                {uint8_t reg = opcodes[pc+2];
                registers[reg] = GPIOA->INDR & 0xFF;
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case READ_BANK_D_REG_X:
                /*
                    Read D bank

                    Read the D bank (PA8-PA15), insert the byte into register X
                */
                {uint8_t reg = opcodes[pc+2];
                registers[reg] = (GPIOA->INDR >> 8) & 0xFF;
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Delay */
            case DELAY_NOPS_X:
                /*
                    Delay nops

                    Delay a number of machine nops. This does not take into account overhead of the VM, so delay may be a bit longer. 

                    If a longer delay than 65535 nops is required, use delay ticks, micros or delay millis

                    Todo: do the math on how long one nop is
                */
                {uint16_t nop_count = *(uint16_t*)(opcodes + pc + 2);
                for (int i = 0; i < nop_count; i++) {
                    __NOP();
                }
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case DELAY_TICKS_X:
                /*
                    Delay ticks

                    Delay a number of ticks, as per the Delay functions.  This does not take into account overhead of the VM, so delay may be a bit longer. 

                    If a longer delay than 65535 ticks is required, use delay micros or delay millis

                    Todo: do the math on how long one tick is
                */
                {uint16_t tick_count = *(uint16_t*)(opcodes + pc + 2);
                Delay_Ticks(tick_count);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case DELAY_MICROS_X:
                /*
                    Delay micros

                    Delay microseconds. This does not take into account overhead of the VM, so delay may be a bit longer. 

                    If a longer delay than 65535 microseconds is required, use delay millis
                */
                {uint16_t microseconds = *(uint16_t*)(opcodes + pc + 2);
                Delay_Us(microseconds);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            case DELAY_MILLIS_X:
                /*
                    Delay millis

                    Delay miliseconds. This does not take into account overhead of the VM, so delay may be a bit longer. 
                */
                {uint16_t milliseconds = *(uint16_t*)(opcodes + pc + 2);
                Delay_Ms(milliseconds);
                pc+=BYTES_PER_INSTRUCTION;
                break;}
            /* Control flow */
            case CALL_X:
                /*
                    Call

                    Function call. Jump to the call address, pushing the address of the following instruction on the stack.
                
                */
                {uint16_t jump_address = *(uint16_t*)(opcodes + pc + 2);
                //Save the address of the next instruction onto the stack
                stack[sp++] = pc + BYTES_PER_INSTRUCTION;
                //Set the PC to the jump address
                pc = jump_address;
                //Continue skips incrementing the PC at the end of the loop
                break;}
            case RET:
                /*
                    Ret

                    Function return. Pop the return address off the top of the stack then jump to it.
                */
                //Pop the return address off the stack
                {
                uint32_t link = stack[--sp];
                //Jump to the address
                pc = link;
                //Continue skips incrementing the PC at the end of the loop
                break;}
            /* Register manip */
            case ASSIGN_REG_X_VALUEOF_REG_Y:
                /*
                    Assign

                    Copy the value of register Y and place it into register X
                
                */
                {uint8_t destination_reg = opcodes[pc+2];
                uint8_t source_reg = opcodes[pc+3];
                registers[destination_reg] = registers[source_reg];
                pc += BYTES_PER_INSTRUCTION;
                break;}
            /* Pile functions */
            case PUSH_UP_PILE_R_FROM_PIN_X:
                /*
                    Push up pile R

                    Read the value at pin X. Push this value onto the r pile (r pile is receive only). Pushing values starts from the bottom up.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint32_t index = *rp;
                uint32_t bit = index % 8;
                uint32_t byte = index / 8;
                uint32_t read = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;
                uint8_t* lipo = pile_r;

                uint8_t mask = 1u << bit;

                if (read) {
                    lipo[byte] |= mask;
                } else {
                    lipo[byte] &= ~mask;
                }

                *rp += 1;
                pc += BYTES_PER_INSTRUCTION;
                
                break;}
            case PUSH_DOWN_PILE_R_FROM_PIN_X:
                /*
                    Push down pile R

                    Read the value at pin X. Push this value onto the r pile (r pile is receive only). Pushing values starts from the top down.
                */
                {uint8_t pin_number = opcodes[pc+2];
                uint32_t index = PILE_SIZE * 8 - 1 - *rp;
                uint32_t bit = index % 8;
                uint32_t byte = index / 8;
                uint32_t read = (GPIOA->INDR & (1 << pin_number)) != (uint32_t)Bit_RESET;
                uint8_t* lipo = pile_r;

                uint8_t mask = 1u << bit;

                if (read) {
                    lipo[byte] |= mask;
                } else {
                    lipo[byte] &= ~mask;
                }

                *rp += 1;
                pc += BYTES_PER_INSTRUCTION;
                
                break;}
            case POP_UP_PILE_T_INTO_PIN_X:
                /*
                    pop up pile T

                    Pop the value at the top of pile T (pile t is transmit only). Write this value to pin X. Popping values towards the bottom of the stack.
                */
                {
                uint8_t pin_number = opcodes[pc+2];
                *tp -= 1;
                uint32_t index = *tp;
                uint32_t bit = index % 8;
                uint32_t byte = index / 8;
                uint8_t* lipo = pile_t;

                uint32_t write = lipo[byte] & (1 << bit);

                if (write) {
                    GPIOA->BSHR = 1 << pin_number;
                } else {
                    GPIOA->BCR = 1 << pin_number;
                }

                pc += BYTES_PER_INSTRUCTION;
                
                break;}
            case POP_DOWN_PILE_T_INTO_PIN_X:
                /*
                    pop down pile T

                    Pop the value at the top of pile T (pile t is transmit only). Write this value to pin X. Popping values towards the top of the stack.
                */
                {
                uint8_t pin_number = opcodes[pc+2];
                *tp -= 1;
                uint32_t index = PILE_SIZE * 8 - 1 - *tp;
                uint32_t bit = index % 8;
                uint32_t byte = index / 8;
                uint8_t* lipo = pile_t;

                uint32_t write = lipo[byte] & (1 << bit);

                if (write) {
                    GPIOA->BSHR = 1 << pin_number;
                } else {
                    GPIOA->BCR = 1 << pin_number;
                }

                pc += BYTES_PER_INSTRUCTION;
                
                break;}

            /* Stop the VM */
            case EXIT:
                /*
                    Exit

                    Stop the VM
                */
                return;
        }
    }
}
