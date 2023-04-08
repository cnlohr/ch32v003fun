/*
 * Example for using I2C with 128x32 graphic OLED
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
	Delay_Ms( 100 );	// give OLED some more time
	printf("initializing i2c oled...");
	if(!oled_init())
	{
		//memset(oled_buffer, 0, sizeof(oled_buffer));
		
		//for(int i=0;i<sizeof(oled_buffer);i++)
		//	oled_buffer[i] = i;
		
		//for(int i=0;i<OLED_W;i++)
		//{
		//	oled_drawPixel(i, i>>2, 1);
		//	oled_drawPixel(i, OLED_H-1-(i>>2), 1);
		//}
		
		//uint8_t y= 0;
		//for(uint8_t x=0;x<OLED_W;x+=16)
		//{
		//	oled_drawLine(x, 0, OLED_W, y, 1);
		//	oled_drawLine(OLED_W-x, OLED_H, 0, OLED_H-y, 1);
		//	y+= 4;
		//}
		
		//for(uint8_t x=0;x<OLED_W;x+=16)
		//	if(x<64)
		//		oled_drawCircle(x,16, 15, 1);
		//	else
		//		oled_fillCircle(x,16, 15, 1);
		
		oled_drawstr(0,0, "This is a test", 1);
		oled_drawstr(0,8, "of the emergency", 1);
		oled_drawstr(0,16,"broadcasting", 1);
		oled_drawstr(0,24,"system.",1);
		
		oled_xorrect(64, 0, 64, 32);
		
		oled_refresh();
		printf("done.\n\r");
	}
	else
		printf("failed.\n\r");
	
	printf("looping...\n\r");
	while(1)
	{
		count++;
		count &= 255;
		Delay_Ms( 5 );
	}
}
