#include "micdeck_periph.h"

// APB1 clock frequency
#define APB1_FREQ          84000000
// Microphone sampling frequency
#define SAMPLE_FREQ        9500

#define DMA_Str DMA2_Stream4
#define DMA_IRQ DMA2_Stream4_IRQn

// {.periph= RCC_AHB1Periph_GPIOA, .port= GPIOA, .pin=GPIO_Pin_7,  .adcCh=ADC_Channel_7}, /* MOSI */

#define IO_PIN       GPIO_Pin_7
#define IO_PORT      GPIOA
#define IO_PORT_CLK  RCC_AHB1Periph_GPIOA

#define ADC_CLK      RCC_APB2Periph_ADC1
#define ADC_NUM      ADC1
#define ADC_CHANNEL  ADC_Channel_7

void TIM_init(void)
{
   TIM_DeInit(TIM3);
   // TIM3 Periph clock enable
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

   // Time base configuration
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
   TIM_TimeBaseStructure.TIM_Period = (int)(APB1_FREQ/SAMPLE_FREQ);
   TIM_TimeBaseStructure.TIM_Prescaler = 0;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   // Setup TIM3 as output source
   TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
   // Enable TIM3
   TIM_Cmd(TIM3, ENABLE);
}

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

void DMA_init(volatile uint16_t *ADCConvertedValue, uint8_t SAMPLES_PER_PACKET)
{
  DMA_DeInit(DMA_Str);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
  DMA_InitTypeDef DMA_InitStructure;
  DMA_StructInit(&DMA_InitStructure);

  // Configure DMA2 Stream 4
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR; //Source address
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &(ADCConvertedValue[0]); //Destination address
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = SAMPLES_PER_PACKET * 2; //Buffer size
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // source size 16bit
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // destination size 16b
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA_Str, &DMA_InitStructure);

  DMA_ITConfig(DMA_Str, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA_Str, DMA_IT_HT, ENABLE);
  DMA_Cmd(DMA_Str, ENABLE);

  // Enable DMA2 IRQ Channel
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
