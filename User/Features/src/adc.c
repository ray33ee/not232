#include "Features/inc/adc.h"


void adc_init(void)
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


uint16_t adc_read(uint8_t pin_number)
{
    uint8_t adc_channel;

    if (pin_number < 8) {
        adc_channel = pin_number;
    } else {
        adc_channel = pin_number - 8;
    }

    ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_55Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    uint16_t v = ADC_GetConversionValue(ADC1);

    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    return v;
}