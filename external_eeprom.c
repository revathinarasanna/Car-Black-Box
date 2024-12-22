/* 
 * File:   EEPROM.c
 * Author: N Revathi
 *
 * Created on 15 November, 2024, 5:35 PM
 */



#include "main.h"
#include "external_eeprom.h"
#include "i2c.h"
#include <xc.h>

void write_eeprom(unsigned char address, unsigned char data)
{
	i2c_start();
	i2c_write(SLAVE_WRITE_E);
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
    for(int i = 3000;i--;);
}

unsigned char read_eeprom(unsigned char address)
{
	unsigned char data;

	i2c_start();
	i2c_write(SLAVE_WRITE_E);
	i2c_write(address);
	i2c_rep_start();
	i2c_write(SLAVE_READ_E);
	data = i2c_read();
	i2c_stop();
  
	return data;
}
