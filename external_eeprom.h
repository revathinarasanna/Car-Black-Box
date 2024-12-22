/* 
 * File:   external_eeprom.h
 * Author: N Revathi
 *
 * Created on 15 November, 2024, 5:36 PM
 */

#ifndef EXTERNAL_EEPROM_H
#define	EXTERNAL_EEPROM_H


#define SLAVE_READ_E		0xA1
#define SLAVE_WRITE_E	    0xA0


void init_ds1307( void);

void write_eeprom(unsigned char address,  unsigned char data);
unsigned char read_eeprom(unsigned char address);


#endif	/* EXTERNAL_EEPROM_H */

