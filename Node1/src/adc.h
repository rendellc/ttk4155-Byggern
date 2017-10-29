///@file
/**
 * Defines an interface for external AD converter
 * using memory mapped IO.
 * NB: AD conversion is only initated when requested, 
 * and this is a blocking process.
 */ 

#ifndef ADC_H_
#define ADC_H_

#include "global_declarations.h"

#define ADC_ADR 0x1100

/**
 * Initialize mcu to interface with adc
 * with Port D pin 2. 
 */
void adc_init(void);

/**
 * Read analog signal at specified channel.
 * No input validation is performed, 
 * @param[in] ch uint8 in range 0 through 3. Channel that adc will perform conversion on.
 * @retval result of AD conversion
 */
uint8_t adc_read_channel(uint8_t);



#endif /* ADC_H_ */