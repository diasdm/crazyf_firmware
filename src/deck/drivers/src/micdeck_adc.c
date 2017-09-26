#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx.h"
#include "micdeck_adc.h"

// {.periph= RCC_AHB1Periph_GPIOA, .port= GPIOA, .pin=GPIO_Pin_7,  .adcCh=ADC_Channel_7}, /* MOSI */

#define IO_PIN       GPIO_Pin_7
#define IO_PORT      GPIOA
#define IO_PORT_CLK  RCC_AHB1Periph_GPIOA

#define ADC_CLK      RCC_APB2Periph_ADC1
#define ADC_NUM      ADC1
#define ADC_CHANNEL  ADC_Channel_7


void ADC_init(void)
{
   // Enable peripheral clocks
   RCC_AHB1PeriphClockCmd(IO_PORT_CLK, ENABLE);
   RCC_APB2PeriphClockCmd(ADC_CLK, ENABLE);

   // Configure ADC1 as analog input
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = IO_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
   GPIO_Init(IO_PORT, &GPIO_InitStructure);

   // ADC Common configuration
   ADC_CommonInitTypeDef ADC_CommonInitStructure;
   ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
   // APB2 clock is half the 168Mhz system clock (i.e. 84Mhz),
   // so with a div by 8, ADC PCLK would be 10.5Mhz
   // STM32F4 datasheet says ADC clock freq should be 0.6Mhz - 30Mhz
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
   ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&ADC_CommonInitStructure);

   // ADC1 initializing structure
   ADC_InitTypeDef ADC_InitStructure;
   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = DISABLE; // 1 Channel
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfConversion = 1;
   ADC_Init(ADC1, &ADC_InitStructure);

   ADC_RegularChannelConfig(ADC_NUM, ADC_CHANNEL, 1, ADC_SampleTime_144Cycles);

   // Enable ADC1 DMA
   ADC_DMACmd(ADC_NUM, ENABLE);
   ADC_DMARequestAfterLastTransferCmd(ADC_NUM, ENABLE);

   // Enable ADC1
   ADC_Cmd(ADC_NUM, ENABLE);
}
