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
			for(uint8_t mode=0;mode<6;mode++)
			{
				// clear buffer for next mode
				oled_setbuf(0);

				switch(mode)
				{
					case 0:
						for(int i=0;i<sizeof(oled_buffer);i++)
							oled_buffer[i] = i;
						break;
					
					case 1:
						for(int i=0;i<OLED_W;i++)
						{
							oled_drawPixel(i, i>>2, 1);
							oled_drawPixel(i, OLED_H-1-(i>>2), 1);
						}
						break;
					
					case 2:
						{
							uint8_t y= 0;
							for(uint8_t x=0;x<OLED_W;x+=16)
							{
								oled_drawLine(x, 0, OLED_W, y, 1);
								oled_drawLine(OLED_W-x, OLED_H, 0, OLED_H-y, 1);
								y+= 4;
							}
						}
						break;
						
					case 3:
						for(uint8_t x=0;x<OLED_W;x+=16)
							if(x<64)
								oled_drawCircle(x,16, 15, 1);
							else
								oled_fillCircle(x,16, 15, 1);
						break;
					
					case 4:
						oled_drawstr(0,0, "This is a test", 1);
						oled_drawstr(0,8, "of the emergency", 1);
						oled_drawstr(0,16,"broadcasting", 1);
						oled_drawstr(0,24,"system.",1);
						
						oled_xorrect(64, 0, 64, 32);
						break;
					case 5:
						oled_drawstr_sz(0,0, "Size 8x8", 1, fontsize_8x8);
						oled_drawstr_sz(0,16, "Size 16x16", 1, fontsize_16x16);
						Delay_Ms(1000);
						oled_refresh();
						oled_drawstr_sz(0,0, "Size 32x32", 1, fontsize_32x32);
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
