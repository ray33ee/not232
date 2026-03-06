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

static inline void capture_pulse(uint32_t ext_line) {
	if(EXTI_GetITStatus(ext_line)!=RESET) {

		uint16_t duration = pulseio_in_tim4_stop();

		pio_buffer[pio_head] = duration;

		pio_head = (pio_head + 1) % PIO_RING_SIZE;

		if (pio_count < PIO_RING_SIZE) {
			pio_count++;
		}

		EXTI_ClearITPendingBit(ext_line);

		pulseio_in_tim4_start();
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