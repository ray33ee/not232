#include "Features/inc/gpio.h"


void gpio_init_default() {
    
    GPIO_InitTypeDef all;

    all.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    all.GPIO_Speed = GPIO_Speed_50MHz;

    //All pins PA0-PA15 
    all.GPIO_Pin = 0xFFFF;

    GPIO_Init(GPIOA, &all);
    
    //Pins PB0, PB1, Pb14 and PB15 
    all.GPIO_Pin = 0xC003;

    GPIO_Init(GPIOB, &all);
}

void gpio_init_port(GPIO_TypeDef* gpio, uint8_t pin, GPIOMode_TypeDef mode) {
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

void gpio_init_ad_pins(uint8_t pin, GPIOMode_TypeDef mode)
{
    gpio_init_port(GPIOA, pin, mode);
}

void gpio_init_f_pins(uint8_t pin, GPIOMode_TypeDef mode)
{
    // Remap pins 16, 17, 18, 19 to 0, 1, 14, 15
    if (pin < 16) {
        return;
    } else if (pin < 18) {
        pin -= 16;
    } else if (pin < 20) {
        pin -= 4;
    } else {
        return;
    }

    gpio_init_port(GPIOB, pin, mode);
}

void gpio_init_adf_pins(uint8_t pin, GPIOMode_TypeDef mode) {
    if (pin < 16) {
        gpio_init_ad_pins(pin, mode);
    } else {
        gpio_init_f_pins(pin, mode);
    }
}

uint32_t gpio_read_adf_pin(uint8_t pin) {
    if (pin < 16) {
        return (GPIOA->INDR & (1 << pin)) != (uint32_t)Bit_RESET;
    } else if (pin < 18) {
        return (GPIOB->INDR & (1 << (pin - 16))) != (uint32_t)Bit_RESET;
    } else if (pin < 20) {
        return (GPIOB->INDR & (1 << (pin - 4))) != (uint32_t)Bit_RESET;
    }
    return 0;
}

void gpio_set_adf_pin(uint8_t pin) {
    if (pin < 16) {
        GPIOA->BSHR = 1 << pin;
    } else if (pin < 18) {
        GPIOB->BSHR = 1 << (pin - 16);
    } else if (pin < 20) {
        GPIOB->BSHR = 1 << (pin - 4);
    }
}

void gpio_clear_adf_pin(uint8_t pin) {
    if (pin < 16) {
        GPIOA->BCR = 1 << pin;
    } else if (pin < 18) {
        GPIOB->BCR = 1 << (pin - 16);
    } else if (pin < 20) {
        GPIOB->BCR = 1 << (pin - 4);
    }
}
