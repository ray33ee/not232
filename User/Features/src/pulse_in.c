#include "Features/inc/pulse_in.h"

#include "Features/inc/gpio.h"


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

/*
 * TIM4 used as a 1 MHz free-running timebase for pulseio (PulseIn)
 */

static inline uint16_t tim4_compute_prescaler_1mhz(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

    uint32_t pclk1  = clocks.PCLK1_Frequency;
    uint32_t timclk = pclk1;

    /* If APB1 prescaler != 1, timer clock doubles (STM32/CH32 behavior) */
    uint32_t ppre1 = (RCC->CFGR0 >> 8u) & 0x7u;   /* PPRE1 bits */
    if (ppre1 >= 0x4u) {
        timclk = pclk1 * 2u;
    }

    /* Prescale to 1 MHz tick */
    return (uint16_t)((timclk / 1000000u) - 1u);
}


void pulseio_in_init(void)
{
    //Setup TIM4 with a 1us tick. pulseio in uses exti to measure and record the time between pulses 
    TIM_TimeBaseInitTypeDef tb = {0};
    tb.TIM_Period            = 0xFFFFu;                       /* free-run */
    tb.TIM_Prescaler         = tim4_compute_prescaler_1mhz(); /* 1 us tick */
    tb.TIM_ClockDivision     = TIM_CKD_DIV1;
    tb.TIM_CounterMode       = TIM_CounterMode_Up;
    tb.TIM_RepetitionCounter = 0u;                            /* ignored on TIM4 */

    TIM_TimeBaseInit(TIM4, &tb);

    /* Optional: ARR preload (harmless with fixed ARR) */
    TIM_ARRPreloadConfig(TIM4, ENABLE);

    /* Ensure timer is stopped after init */
    TIM_Cmd(TIM4, DISABLE);

    /* Clear flags */
    TIM_ClearFlag(TIM4, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);
}


void pulseio_in_tim4_start(void)
{
    /* Reset counter and clear any pending flags before starting */
    TIM_SetCounter(TIM4, 0u);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

    TIM_Cmd(TIM4, ENABLE);
}

uint16_t pulseio_in_tim4_stop(void)
{
    /* Read current counter value first, then stop the timer */
    uint16_t cnt = (uint16_t)TIM4->CNT;
    TIM_Cmd(TIM4, DISABLE);
    return cnt;
}

void pulseio_in_resume(uint8_t pin, uint32_t trigger_duration) {
    if (trigger_duration != 0) {

        gpio_init_ad_pins(pin, GPIO_Mode_Out_OD);

        GPIOA->BCR = 1 << pin;

        Delay_Us(trigger_duration);
        
        GPIOA->BSHR = 1 << pin;
    }


    gpio_init_ad_pins(pin, GPIO_Mode_IN_FLOATING);

    //Setup the EXTI on the pin
    EXTI_INIT(pin);

    //Start timer
    pulseio_in_tim4_start();
}

void pulseio_in_clear() {
    __disable_irq();
    pio_head = 0;
    pio_count = 0;
    __enable_irq();
}

void pulseio_in_stop(uint8_t pin) {

    EXTI_DEINIT(pin);

    pulseio_in_tim4_stop();
}

void pulseio_in_read(uint32_t try_read) {
    __disable_irq();

    uint32_t pio_head_copy = pio_head;
    uint32_t pio_count_copy = pio_count;

    __enable_irq();
    
    uint32_t modified_count;

    if (pio_count_copy < try_read) {
        modified_count = pio_count_copy;
    } else {
        modified_count = try_read;
    }

    usbSerial_blocking_writeP_u32(modified_count);

    uint32_t tail = (pio_head_copy + PIO_RING_SIZE - modified_count) % PIO_RING_SIZE;

    for (int i = 0; i < modified_count; i++) {
        uint16_t value = pio_buffer[(tail + i) % PIO_RING_SIZE];
        usbSerial_blocking_writeP_u32(value);
    }
}
