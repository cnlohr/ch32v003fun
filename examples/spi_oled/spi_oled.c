/*
 * Example for using SPI with 128x32 graphic OLED
 * 03-29-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdlib.h>
#include "oled.h"

/* White Noise Generator State */
#define NOISE_BITS 8
#define NOISE_MASK ((1<<NOISE_BITS)-1)
#define NOISE_POLY_TAP0 31
#define NOISE_POLY_TAP1 21
#define NOISE_POLY_TAP2 1
#define NOISE_POLY_TAP3 0
uint32_t lfsr = 1;

/*
 * random byte generator
 */
uint8_t rand8(void)
{
	uint8_t bit;
	uint32_t new_data;
	
	for(bit=0;bit<NOISE_BITS;bit++)
	{
		new_data = ((lfsr>>NOISE_POLY_TAP0) ^
					(lfsr>>NOISE_POLY_TAP1) ^
					(lfsr>>NOISE_POLY_TAP2) ^
					(lfsr>>NOISE_POLY_TAP3));
		lfsr = (lfsr<<1) | (new_data&1);
	}
	
	return lfsr&NOISE_MASK;
}

/*
 * return pixel value in buffer
 */
uint8_t getpix(uint8_t *buf, int16_t x, int16_t y)
{
	if((x<0) || (x>=OLED_W))
		return 0;
	if((y<0) || (y>=OLED_H))
		return 0;
	return buf[x+OLED_W*(y>>3)] & (1<<(y&7)) ? 1 : 0;
}

/*
 * conway's life on B/W OLED
 */
void conway(uint8_t *buf)
{
	uint8_t col[2][(OLED_H>>3)];
	int16_t x, y, B, b, sum, d, colidx = 0;
	
	/* iterate over columns */
	for(x=0;x<OLED_W;x++)
	{
		/* iterate bytes in column */
		for(B=0;B<(OLED_H>>3);B++)
		{
			/* init byte accum */
			d = 0;
			
			/* iterate over bits in byte */
			for(b=0;b<8;b++)
			{
				/* which row */
				y = B*8+b;
				
				/* prep byte accum for this bit */
				d >>= 1;
				
				/* count live neighbors */
				sum = getpix(buf, x-1, y-1)+getpix(buf, x, y-1)+getpix(buf, x+1, y-1)+
					getpix(buf, x-1, y)+getpix(buf, x+1, y)+
					getpix(buf, x-1, y+1)+getpix(buf, x, y+1)+getpix(buf, x+1, y+1);
				
				/* check life for next cycle */
				if(getpix(buf, x, y))
				{
					/* live cell */
					if((sum==2)||(sum==3))
						d |= 128;
				}
				else
				{
					/* dead cell */
					if(sum == 3)
						d |= 128;
				}
				
				//printf("x=%3d, B=%1d, b=%1d, y=%2d\n\r", x, B, b, y);
			}

			col[colidx][B] = d;
		}
		
		colidx ^= 1;
		
		if(x>0)
		{
			/* update previous column */
			for(y=0;y<(OLED_H>>3);y++)
				buf[(x-1)+OLED_W*y] = col[colidx][y];
		}
	}
	
	colidx ^= 1;

	/* update final column */
	for(y=0;y<(OLED_H>>3);y++)
		buf[127+OLED_W*y] = col[colidx][y];
}

int count = 0;

int main()
{
	// 48MHz internal clock
	SystemInit48HSI();

	// start serial @ default 115200bps
#ifdef STDOUT_UART
	SetupUART( UART_BRR );
	Delay_Ms( 100 );
#else
	SetupDebugPrintf();
#endif
	printf("\r\r\n\nspi_oled example\n\r");

	// init spi and oled
	Delay_Ms( 100 );	// give OLED some more time
	printf("initializing spi oled...");
	if(!oled_init())
	{
		printf("done.\n\r");
		
#if 0
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
			
				printf("count = %d\n\r", count++);
				
				Delay_Ms(2000);
			}
		}
#else
		printf("Looping...\n\r");
		while(1)
		{
			int i;
			
			/* fill with random */
			for(i=0;i<sizeof(oled_buffer);i++)
			{
				oled_buffer[i] = rand8();
			}
			oled_refresh();

			/* run conway iterations */
			for(i=0;i<500;i++)
			{
				conway(oled_buffer);
				
				/* refresh */
				oled_refresh();
			}
			
			printf("count = %d\n\r", count++);
		
			Delay_Ms(2000);
		}
#endif
	}
	else
		printf("failed.\n\r");
	
	printf("Stuck here forever...\n\r");
	while(1);
}
