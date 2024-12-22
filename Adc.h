/* 
 * File:   Adc.h
 * Author: N Revathi
 *
 * Created on 13 November, 2024, 6:38 PM
 */

#ifndef ADC_H
#define	ADC_H

#define CHANNEL4		0x04
void init_adc(void);
unsigned short read_adc(unsigned char channel);

#endif	/* ADC_H */

