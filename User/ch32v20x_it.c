/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v20x_it.h"

uint32_t pio_head = 0;
uint32_t pio_count = 0;
uint16_t pio_buffer[PIO_RING_SIZE];

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void TIM4_pulseio_start(void)
{
    /* Reset counter and clear any pending flags before starting */
    TIM_SetCounter(TIM4, 0u);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

    TIM_Cmd(TIM4, ENABLE);
}

uint16_t TIM4_pulseio_stop(void)
{
    /* Read current counter value first, then stop the timer */
    uint16_t cnt = (uint16_t)TIM4->CNT;
    TIM_Cmd(TIM4, DISABLE);
    return cnt;
}

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  NVIC_SystemReset();
  while (1)
  {
  }
}

void capture_pulse(uint32_t ext_line) {
	if(EXTI_GetITStatus(ext_line)!=RESET) {

		uint16_t duration = TIM4_pulseio_stop();

		pio_buffer[pio_head] = duration;

		pio_head = (pio_head + 1) % PIO_RING_SIZE;

		if (pio_count < PIO_RING_SIZE) {
			pio_count++;
		}

		EXTI_ClearITPendingBit(ext_line);

		TIM4_pulseio_start();
	}
}

void EXTI0_IRQHandler(void)
{
	capture_pulse(EXTI_Line0);
}

void EXTI1_IRQHandler(void)
{
	capture_pulse(EXTI_Line1);
}

void EXTI2_IRQHandler(void)
{
	capture_pulse(EXTI_Line2);
}

void EXTI3_IRQHandler(void)
{
	capture_pulse(EXTI_Line3);
}

void EXTI4_IRQHandler(void)
{
	capture_pulse(EXTI_Line4);
}

void EXTI9_5_IRQHandler(void) {
	capture_pulse(EXTI_Line5);
	capture_pulse(EXTI_Line6);
	capture_pulse(EXTI_Line7);
	capture_pulse(EXTI_Line8);
	capture_pulse(EXTI_Line9);
}

void EXTI15_10_IRQHandler(void) {
	capture_pulse(EXTI_Line10);
	capture_pulse(EXTI_Line11);
	capture_pulse(EXTI_Line12);
	capture_pulse(EXTI_Line13);
	capture_pulse(EXTI_Line14);
	capture_pulse(EXTI_Line15);
}