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
#include "Features/inc/pwm.h"
#include "Features/inc/adc.h"
#include "Features/inc/pulse_in.h"
/*

PLEASE SEE THIS LINK

https://github.com/guuuuus/ch32v20x_usbSerial

It contains an implementation of USB-CDC, read and write, which works with Moun River

*/

/* Global typedef */

/* Global define */



/* Global Variable */

/*
    Setup the timer for 37.5KHz, 8-bit pwm on all 4 channels
*/


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

    /* Set all GPIOA pins (and the 4 GPIOB) to floating inputs*/
    gpio_init_default();
    
    /* Setup PWM timers */
    pwm_init();

    /* Setup ADC */
    adc_init();

    /* Setup pulseio timer */
    pulseio_in_init();

    printf("Starting\r\n");
    
    while(1)
    {

        if (usbSerial_available()) {
            get_packet();

        }


        usbSerial_flush();
    }
}
