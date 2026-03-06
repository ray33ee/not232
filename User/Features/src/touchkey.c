#include "Features/inc/touchkey.h"

static inline void tkey_enable(void)
{
    TKey1->CTLR1 |= (1U << 24) | (1U << 26);
}

static inline void tkey_disable(void)
{
    TKey1->CTLR1 &= ~((1U << 24) | (1U << 26));
}

uint16_t tkey_read(uint8_t pin_number)
{

    uint8_t adc_channel;

    if (pin_number <= 7) {
        adc_channel = pin_number;          // PA0..PA7 -> ADC0..ADC7
    } else if (pin_number == 16) {
        adc_channel = 8;                   // PB0 -> ADC8
    } else if (pin_number == 17) {
        adc_channel = 9;                   // PB1 -> ADC9
    } else {
        return 0; // invalid TouchKey pin
    }

    tkey_enable();

    ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_7Cycles5);
    TKey1->IDATAR1 = 0x10;
    TKey1->RDATAR  = 0x08;

    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {}

    uint16_t value = (uint16_t)TKey1->RDATAR;

    tkey_disable();
    return value;
}