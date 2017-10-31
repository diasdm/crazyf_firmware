#define DEBUG_MODULE "MICDECK"

#include <stdint.h>

#include "debug.h"
#include "deck.h"
#include "crtp.h"
#include "console.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "stm32f4xx.h"
#include "micdeck_periph.h"

// Sets the port to the first unused port
#define STREAM_PORT 0x01
// The stream channel value is not relevant
#define STREAM_CHANNEL 0
// Number of bytes of data sent
#define DATA_BYTES 29 // Number of bytes which contain data
#define COUNT_BYTES 1 // Number of bytes used to track the packets
// Number of microphone samples per packet
#define SAMPLES_PER_PACKET 19

// Packet to be sent
static CRTPPacket p;
// Packet count
static uint8_t packetCount = 0;
// Data buffer pointer
uint8_t ptr = 0;
// Is in middle of byte flag
uint8_t byteHalf = 0;
// Auxiliar values
uint16_t sample = 0;
uint8_t i = 0;
uint8_t end = 0;
// Vector where the sample values are written to
volatile uint16_t ADCConvertedValue[SAMPLES_PER_PACKET*2];

/*
 * Packs and send a CRTP packet
 * uint8_t section - Is 0 if we are accessing the first half of the ADCConvertedValue
 * 		        1 if we are accessing the second half of the ADCConvertedValue
 */
void packsData(uint8_t section) {
    // Places the packetCount in the first available byte
    p.data[0] = packetCount;
    // Increases the packet count
    packetCount++;
    // Initializes "pointer"
    ptr = 1;
    // Variable which let us know if we are in the middle of a byte or not
    byteHalf = 0;
    // Computes start and end position for the sample selecting
    i = SAMPLES_PER_PACKET * section;
    end = SAMPLES_PER_PACKET * (section + 1);
    // Packs samples
    do{
	sample = ADCConvertedValue[i];
	if(byteHalf){
	    p.data[ptr] |= (uint8_t)(sample >> 8);
	    ptr++;
	    p.data[ptr] = (uint8_t)(sample);
	    ptr++;
	    byteHalf = 0;
	}else{
	    p.data[ptr] = (uint8_t)(sample >> 4);
	    ptr++;
	    p.data[ptr] = (uint8_t)(sample << 4);
	    byteHalf = 1;
	}
	i++;
    }while(i != end);
    // Sends packet
    crtpSendPacketISR(&p);
    // Clears memory
    memset(&(p.data[1]), 0, DATA_BYTES);
}

/*
 * Catches DMA interruption, calls the packing function and clears flag
 */
void DMA2_Stream4_IRQHandler(void)
{
  // DMA interruption for first half of the vector
  if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_HTIF4)) {
    packsData(0);
    DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_HTIF4);
  }
  // DMA interruption for second half of the vector
  if (DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF4)) {
    packsData(1);
    DMA_ClearITPendingBit(DMA2_Stream4, DMA_IT_TCIF4);
  }
}

/*
 * Deck main function, initializes packet heather and peripherals
 */
static void micDeckInit (DeckInfo *info) {
  DEBUG_PRINT("MicDeck initializing.\n");
  p.header = CRTP_HEADER(STREAM_PORT, STREAM_CHANNEL);
  p.size = DATA_BYTES + COUNT_BYTES;

  ADC_init();
  DMA_init(ADCConvertedValue, (uint8_t) SAMPLES_PER_PACKET);
  TIM_init();
}

/*
 * Deck test function
 */
static bool micDeckTest()
{
  DEBUG_PRINT("MicDeck tests passed!\n");
  return true;
}

/*
 * Deck specification
 */
static const DeckDriver micDeckDriver = {
  .name = "micDeck",
  .init = micDeckInit,
  .test = micDeckTest,
  .usedGpio = DECK_USING_MOSI,
  .usedPeriph = DECK_USING_TIMER3,
};

DECK_DRIVER(micDeckDriver);
