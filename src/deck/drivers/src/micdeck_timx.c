#include "stm32f4xx.h"
#include "micdeck_timx.h"

#define APB1_FREQ          84000000
#define SAMPLE_FREQ        9500

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
