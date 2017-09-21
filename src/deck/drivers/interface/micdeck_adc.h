#ifndef SRC_DECK_DRIVERS_INTERFACE_MICDECK_ADC_H_
#define SRC_DECK_DRIVERS_INTERFACE_MICDECK_ADC_H_

#include <stdint.h>

void ADC_init(void);
void ADC_start(void);
uint16_t ADC_get(void);

#endif /* SRC_DECK_DRIVERS_INTERFACE_MICDECK_ADC_H_ */
