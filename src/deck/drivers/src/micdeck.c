#define DEBUG_MODULE "MICDECK"

#include <stdint.h>

#include "debug.h"
#include "deck.h"
#include "crtp.h"
#include "console.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stm32f4xx.h"
#include "micdeck_adc.h"
#include "micdeck_timx.h"
#include "micdeck_dma.h"

// Sets the port to the first unused port
#define STREAM_PORT 0x01
// I believe the channel number isn't relevant but we can use different channels to stream the sound of different microphones
#define STREAM_CHANNEL 0
// Crazyflie GPIO used
#define GPIO_USED DECK_GPIO_TX2
// Number of bytes of data sent
#define DATA_BYTES 29
#define COUNT_BYTES 1
// Number of mic samples in one packet
#define SAMPLES_PER_PACKET 19
// Packet to be sent
CRTPPacket p;
// Number of samples buffered
volatile int sampleNum = 0;
// Packet count
uint8_t packetCount = 0;
// Data buffer pointer
int ptr = 0;
// Is is midle of byte flag
int byteHalf = 0;

static uint8_t dataBuffer[DATA_BYTES];

// Timer loop and handle
static xTimerHandle sendPacketTimer;

volatile uint16_t ADCConvertedValue[SAMPLES_PER_PACKET];

static void micTask(xTimerHandle timer)
{
  if(sampleNum == SAMPLES_PER_PACKET){
    // Copies data to package
    p.data[0] = packetCount;
    memcpy(&(p.data[1]), dataBuffer, DATA_BYTES);
    crtpSendPacket(&p);
    packetCount++;
    taskENTER_CRITICAL();
    sampleNum = 0;
    ptr = 0;
    byteHalf = 0;
    memset(dataBuffer, 0, DATA_BYTES);
    taskEXIT_CRITICAL();
  }
}

void TIM6_DAC_IRQHandler(void)
{
   if (TIM_GetITStatus(TIM6, TIM_IT_Update)) {
      ADC_start();
      TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
   }
}

void DMA2_Stream4_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TC)) {
      for(int i = 0; i < SAMPLES_PER_PACKET; i++) {
	  uint16_t sample = ADCConvertedValue[i];

	  if(!byteHalf){
	    dataBuffer[ptr] = (uint8_t)(sample >> 4);
	    ptr++;
	    dataBuffer[ptr] = (uint8_t)(sample << 4);
	    byteHalf = 1;
	  }else{
	    dataBuffer[ptr] |= (uint8_t)(sample >> 8);
	    ptr++;
	    dataBuffer[ptr] = (uint8_t)(sample);
	    ptr++;
	    byteHalf = 0;
	  }
	  sampleNum++;
      }
    DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TC);
  }
}

static void micDeckInit (DeckInfo *info) {
  DEBUG_PRINT("MicDeck initializing.\n");
  // consoleInit();
  p.header = CRTP_HEADER(STREAM_PORT, STREAM_CHANNEL);
  p.size = DATA_BYTES + COUNT_BYTES;
  // Create and start the send packet timer with a period of 1ms
  sendPacketTimer = xTimerCreate("sendPacketTimer", M2T(4), pdTRUE, NULL, micTask);
  xTimerStart(sendPacketTimer, 100);
  tmr_sample_init();
  ADC_init();
  DMA_init(&(ADCConvertedValue[0]));
  // consoleFlush();
}

static bool micDeckTest()
{
  DEBUG_PRINT("MicDeck tests passed!\n");
  return true;
}

static const DeckDriver micDeckDriver = {
    .name = "micDeck",
  .init = micDeckInit,
  .test = micDeckTest,
  .usedGpio = DECK_GPIO_MOSI,
};

DECK_DRIVER(micDeckDriver);
