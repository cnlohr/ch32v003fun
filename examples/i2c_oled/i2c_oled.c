/*
 * Example for using I2C with graphic OLED
 * 03-29-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "oled.h"

int main()
{
	uint32_t count = 0;
	
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	Delay_Ms( 100 );
	printf("\r\r\n\ni2c_oled example\n\r");

	// init i2c and oled
	printf("initializing i2c oled...");
	oled_init();
	memset(oled_buffer, 0x88, sizeof(oled_buffer));
	oled_refresh();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	while(1)
	{
		count++;
		count &= 255;
		Delay_Ms( 5 );
	}
}
