#include "Features/inc/pwm.h"

//Sets up a timer and all 4 of its channels for 8-bit 37.5KHz pwm (used in the pulseio Out code too)
void pwm_timer_init(TIM_TypeDef* timer) {

    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    // Count up to 255, 37.5KHz frequency
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

    TIM_OC3Init(timer, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(timer, TIM_OCPreload_Disable);

    TIM_OC4Init(timer, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(timer, TIM_OCPreload_Disable);
    
    TIM_ARRPreloadConfig(timer, ENABLE);

    //TIM1 is an advanced timer so needs this line
    if (timer == TIM1) {
        TIM_CtrlPWMOutputs(timer, ENABLE);
    }

    TIM_Cmd(timer, ENABLE);

}

void pwm_init() {
    pwm_timer_init(TIM1);
    pwm_timer_init(TIM2);
    pwm_timer_init(TIM3);
}

// Set the duty cycle from pin number
void pwm_set_duty(uint8_t pin_number, uint8_t duty) {
    uint16_t _duty = (uint16_t)duty;
    if (pin_number == 0) { //A0
        TIM2->CH1CVR = _duty;
    } else if (pin_number == 1) { //A1
        TIM2->CH2CVR = _duty;
    } else if (pin_number == 2) { //A2
        TIM2->CH3CVR = _duty;
    } else if (pin_number == 3) { //A3
        TIM2->CH4CVR = _duty;
    } else if (pin_number == 6) { //A6
        TIM3->CH1CVR = _duty;
    } else if (pin_number == 7) { //A7
        TIM3->CH2CVR = _duty;
    } else if (pin_number == 16) { //B0
        TIM3->CH3CVR = _duty;
    } else if (pin_number == 17) { //B1
        TIM3->CH4CVR = _duty;
    } else if (pin_number == 8) { //D0
        TIM1->CH1CVR = _duty;
    } else if (pin_number == 9) { //D1
        TIM1->CH2CVR = _duty;
    } else if (pin_number == 10) { //D2
        TIM1->CH3CVR = _duty;
    } else if (pin_number == 11) { //D3
        TIM1->CH4CVR = _duty;
    }
}