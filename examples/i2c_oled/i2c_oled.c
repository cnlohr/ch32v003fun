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
	// 48MHz internal clock
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
		printf("done.\n\r");
		
		printf("Looping on test modes...");
		while(1)
		{
			for(uint8_t mode=0;mode<(OLED_H>32?8:7);mode++)
			{
				// clear buffer for next mode
				oled_setbuf(0);

				switch(mode)
				{
					case 0:
						printf("buffer fill with binary\n\r");
						for(int i=0;i<sizeof(oled_buffer);i++)
							oled_buffer[i] = i;
						break;
					
					case 1:
						printf("pixel plots\n\r");
						for(int i=0;i<OLED_W;i++)
						{
							oled_drawPixel(i, i/(OLED_W/OLED_H), 1);
							oled_drawPixel(i, OLED_H-1-(i/(OLED_W/OLED_H)), 1);
						}
						break;
					
					case 2:
						{
							printf("Line plots\n\r");
							uint8_t y= 0;
							for(uint8_t x=0;x<OLED_W;x+=16)
							{
								oled_drawLine(x, 0, OLED_W, y, 1);
								oled_drawLine(OLED_W-x, OLED_H, 0, OLED_H-y, 1);
								y+= OLED_H/8;
							}
						}
						break;
						
					case 3:
						printf("Circles empty and filled\n\r");
						for(uint8_t x=0;x<OLED_W;x+=16)
							if(x<64)
								oled_drawCircle(x, OLED_H/2, 15, 1);
							else
								oled_fillCircle(x, OLED_H/2, 15, 1);
						break;
					
					case 4:
						printf("Unscaled Text\n\r");
						oled_drawstr(0,0, "This is a test", 1);
						oled_drawstr(0,8, "of the emergency", 1);
						oled_drawstr(0,16,"broadcasting", 1);
						oled_drawstr(0,24,"system.",1);
						if(OLED_H>32)
						{
							oled_drawstr(0,32, "Lorem ipsum", 1);
							oled_drawstr(0,40, "dolor sit amet,", 1);
							oled_drawstr(0,48,"consectetur", 1);
							oled_drawstr(0,56,"adipiscing",1);
						}
						oled_xorrect(OLED_W/2, 0, OLED_W/2, OLED_W);
						break;
						
					case 5:
						printf("Scaled Text 1, 2\n\r");
						oled_drawstr_sz(0,0, "sz 8x8", 1, fontsize_8x8);
						oled_drawstr_sz(0,16, "16x16", 1, fontsize_16x16);
						break;
					
					case 6:
						printf("Scaled Text 4\n\r");
						oled_drawstr_sz(0,0, "32x32", 1, fontsize_32x32);
						break;
					
					
					case 7:
						printf("Scaled Text 8\n\r");
						oled_drawstr_sz(0,0, "64", 1, fontsize_64x64);
						break;

					default:
						break;
				}
				oled_refresh();
			
				Delay_Ms(2000);
			}
		}
	}
	else
		printf("failed.\n\r");
	
	printf("Stuck here forever...\n\r");
	while(1);
}
