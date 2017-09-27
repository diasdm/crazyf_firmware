#ifndef SRC_DECK_DRIVERS_INTERFACE_MICDECK_PERIPH_H_
#define SRC_DECK_DRIVERS_INTERFACE_MICDECK_PERIPH_H_

#include <stdint.h>

#include "stm32f4xx.h"

void TIM_init(void);

void ADC_init(void);

void DMA_init(volatile uint16_t *ADCConvertedValue, uint8_t SAMPLES_PER_PACKET);

#endif /* SRC_DECK_DRIVERS_INTERFACE_MICDECK_PERIPH_H_ */
