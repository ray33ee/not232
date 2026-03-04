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
#include "USB_Serial/inc/usb_serial.h"
#include "USB_Serial/inc/ch32v20x_usbfs_device.h"
#include "comms/inc/comms.h"
/*

PLEASE SEE THIS LINK

https://github.com/guuuuus/ch32v20x_usbSerial

It contains an implementation of USB-CDC, read and write, which works with Moun River

*/

/* Global typedef */

/* Global define */



/* Global Variable */

/*
    Setup the timer for 40KHz, 8-bit pwm on all 4 channels (2 channels for TIM3)
*/
void CH_init(TIM_TypeDef* timer) {

    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    // Count up to 255, 40KHz frequency
    TIM_TimeBaseInitStructure.TIM_Period        = 255;
    TIM_TimeBaseInitStructure.TIM_Prescaler     = 14;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timer, &TIM_TimeBaseInitStructure);
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_OC1Init(timer, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(timer, TIM_OCPreload_Disable);

    TIM_OC2Init(timer, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(timer, TIM_OCPreload_Disable);

    //We don't use Channel 3 or 4 for TIM3
    if (timer != TIM3) {
        TIM_OC3Init(timer, &TIM_OCInitStructure);
        TIM_OC3PreloadConfig(timer, TIM_OCPreload_Disable);

        TIM_OC4Init(timer, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(timer, TIM_OCPreload_Disable);
    }
    TIM_ARRPreloadConfig(timer, ENABLE);

    if (timer == TIM1) {
        TIM_CtrlPWMOutputs(timer, ENABLE);
    }

    TIM_Cmd(timer, ENABLE);

}

/*
 * TIM4 used as a 1 MHz free-running timebase for pulseio
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

void TIM4_pulseio_init(void)
{

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

void ADC1_Init_Single(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};

    /* ADC clock */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    ADC_DeInit(ADC1);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    /* No injected mode needed */
    ADC_InjectedDiscModeCmd(ADC1, DISABLE);
    ADC_ExternalTrigInjectedConvCmd(ADC1, DISABLE);

    ADC_Cmd(ADC1, ENABLE);

    /* Calibrate (buffer off during calibration is common on CH32 libs) */
    ADC_BufferCmd(ADC1, DISABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
    ADC_BufferCmd(ADC1, ENABLE);

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

    /* Setup all the peripheral clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* Initialise the USB */
    USBFS_RCC_Init();
    usbSerial_begin();

    while(!usbSerial_connected()) { }

    /* Set all GPIOA pins to floating inputs*/
    GPIO_InitTypeDef all;

    all.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    all.GPIO_Speed = GPIO_Speed_50MHz;
    all.GPIO_Pin = 0xFFFF;

    GPIO_Init(GPIOA, &all);
    
    /* Setup PWM timers */
    CH_init(TIM3);
    CH_init(TIM2);
    CH_init(TIM1);

    /* Setup ADC */
    ADC1_Init_Single();

    /* Setup pulseio timer */
    TIM4_pulseio_init();

    printf("Starting\r\n");
    
    while(1)
    {

        if (usbSerial_available()) {
            get_packet();

        }


        usbSerial_flush();
    }
}
