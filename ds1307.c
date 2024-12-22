
#include "main.h"
#include "i2c.h"
#include "ds1307.h"
#include <xc.h>

#include"clcd.h"
/* 
 * DS1307 Slave address
 * D0  -  Write Mode
 * D1  -  Read Mode
 */

void init_ds1307(void)
{
	unsigned char dummy;

	/* Setting the CH bit of the RTC to Stop the Clock */
	dummy = read_ds1307(SEC_ADDR);
	write_ds1307(SEC_ADDR, dummy & 0x7F); 

	/* Seting 12 Hr Format */
	dummy = read_ds1307(HOUR_ADDR);
	write_ds1307(HOUR_ADDR, dummy & 0xBF); 

	

}

void write_ds1307(unsigned char address, unsigned char data)
{
	i2c_start();
	i2c_write(SLAVE_WRITE);
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
}

unsigned char read_ds1307(unsigned char address)
{
	unsigned char data;
	i2c_start();
	i2c_write(SLAVE_WRITE);
	i2c_write(address);
	i2c_rep_start();
	i2c_write(SLAVE_READ);
	data = i2c_read();
	i2c_stop();
	return data;
}