#include "Features/inc/gpio.h"


/*

This following mappings send the Not232 pin numbers (0-19) to the CH32 GPIO_TypeDef* port and uint16_t pin number

*/
GPIO_TypeDef* pin_port_mapping[PIN_COUNT] = { GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB, GPIOB, GPIOB };
uint8_t pin_number_mapping[PIN_COUNT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 14, 15 };


void gpio_base_init_pins(GPIO_TypeDef* gpio, uint8_t pin, GPIOMode_TypeDef mode) {
    uint16_t mask = 1 << pin;
    uint32_t currentmode = 0x00, pos = 0x00;
    uint32_t tmpreg = 0x00, pinmask = 0x00;

    currentmode = ((uint32_t)mode) & ((uint32_t)0x0F);

    if((((uint32_t)mode) & ((uint32_t)0x10)) != 0x00)
    {
        currentmode |= (uint32_t)GPIO_Speed_50MHz;
    }

    if(mask > 0x00FF) {
        tmpreg = gpio->CFGHR;
        pos = (pin - 8) << 2;
    } else {
        tmpreg = gpio->CFGLR;
        pos = pin << 2;
    }

    pinmask = ((uint32_t)0x0F) << pos;
    tmpreg &= ~pinmask;
    tmpreg |= (currentmode << pos);

    if (mask > 0x00FF) {
        gpio->CFGHR = tmpreg;
    } else {
        gpio->CFGLR = tmpreg;
    }
}

void gpio_init_pin(uint8_t pin, GPIOMode_TypeDef mode) {
    gpio_base_init_pins(pin_port_mapping[pin], pin_number_mapping[pin], mode);
}

void gpio_init_default() {
    //Set all pins 0-19 as floating inputs. 
    //This is probably not needed but anyway
    for (int i = 0; i < PIN_COUNT; i++) {
        gpio_base_init_pins(pin_port_mapping[i], pin_number_mapping[i], GPIO_Mode_IN_FLOATING);
    }

    //Setup PB3 as out pp for the on board WS2812B
    gpio_base_init_pins(GPIOB, 3, GPIO_Mode_Out_PP);

}


uint32_t gpio_read_pin(uint8_t pin) {
    return (pin_port_mapping[pin]->INDR & (1 << pin_number_mapping[pin])) != (uint32_t)Bit_RESET;
}

void gpio_set_pin(uint8_t pin) {
    pin_port_mapping[pin]->BSHR = 1 << pin_number_mapping[pin];
}

void gpio_clear_pin(uint8_t pin) {
    pin_port_mapping[pin]->BCR = 1 << pin_number_mapping[pin];
}
