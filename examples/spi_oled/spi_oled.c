/*
 * Example for using SPI with 128x32 graphic OLED
 * 03-29-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64

#include "ch32v003fun.h"
#include <stdio.h>
#include "ssd1306_spi.h"
#include "ssd1306.h"

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
	if((x<0) || (x>=SSD1306_W))
		return 0;
	if((y<0) || (y>=SSD1306_H))
		return 0;
	return buf[x+SSD1306_W*(y>>3)] & (1<<(y&7)) ? 1 : 0;
}

/*
 * conway's life on B/W OLED
 */
void conway(uint8_t *buf)
{
	uint8_t col[2][(SSD1306_H>>3)];
	int16_t x, y, B, b, sum, d, colidx = 0;
	
	/* iterate over columns */
	for(x=0;x<SSD1306_W;x++)
	{
		/* iterate bytes in column */
		for(B=0;B<(SSD1306_H>>3);B++)
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
			for(y=0;y<(SSD1306_H>>3);y++)
				buf[(x-1)+SSD1306_W*y] = col[colidx][y];
		}
	}
	
	colidx ^= 1;

	/* update final column */
	for(y=0;y<(SSD1306_H>>3);y++)
		buf[127+SSD1306_W*y] = col[colidx][y];
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
	if(!ssd1306_spi_init())
	{
		ssd1306_init();
		printf("done.\n\r");
		
#if 0
		printf("Looping on test modes...");
		while(1)
		{
			for(uint8_t mode=0;mode<(SSD1306_H>32?8:7);mode++)
			{
				// clear buffer for next mode
				ssd1306_setbuf(0);

				switch(mode)
				{
					case 0:
						printf("buffer fill with binary\n\r");
						for(int i=0;i<sizeof(ssd1306_buffer);i++)
							ssd1306_buffer[i] = i;
						break;
					
					case 1:
						printf("pixel plots\n\r");
						for(int i=0;i<SSD1306_W;i++)
						{
							ssd1306_drawPixel(i, i/(SSD1306_W/SSD1306_H), 1);
							ssd1306_drawPixel(i, SSD1306_H-1-(i/(SSD1306_W/SSD1306_H)), 1);
						}
						break;
					
					case 2:
						{
							printf("Line plots\n\r");
							uint8_t y= 0;
							for(uint8_t x=0;x<SSD1306_W;x+=16)
							{
								ssd1306_drawLine(x, 0, SSD1306_W, y, 1);
								ssd1306_drawLine(SSD1306_W-x, SSD1306_H, 0, SSD1306_H-y, 1);
								y+= SSD1306_H/8;
							}
						}
						break;
						
					case 3:
						printf("Circles empty and filled\n\r");
						for(uint8_t x=0;x<SSD1306_W;x+=16)
							if(x<64)
								ssd1306_drawCircle(x, SSD1306_H/2, 15, 1);
							else
								ssd1306_fillCircle(x, SSD1306_H/2, 15, 1);
						break;
					
					case 4:
						printf("Unscaled Text\n\r");
						ssd1306_drawstr(0,0, "This is a test", 1);
						ssd1306_drawstr(0,8, "of the emergency", 1);
						ssd1306_drawstr(0,16,"broadcasting", 1);
						ssd1306_drawstr(0,24,"system.",1);
						if(SSD1306_H>32)
						{
							ssd1306_drawstr(0,32, "Lorem ipsum", 1);
							ssd1306_drawstr(0,40, "dolor sit amet,", 1);
							ssd1306_drawstr(0,48,"consectetur", 1);
							ssd1306_drawstr(0,56,"adipiscing",1);
						}
						ssd1306_xorrect(SSD1306_W/2, 0, SSD1306_W/2, SSD1306_W);
						break;
						
					case 5:
						printf("Scaled Text 1, 2\n\r");
						ssd1306_drawstr_sz(0,0, "sz 8x8", 1, fontsize_8x8);
						ssd1306_drawstr_sz(0,16, "16x16", 1, fontsize_16x16);
						break;
					
					case 6:
						printf("Scaled Text 4\n\r");
						ssd1306_drawstr_sz(0,0, "32x32", 1, fontsize_32x32);
						break;
					
					
					case 7:
						printf("Scaled Text 8\n\r");
						ssd1306_drawstr_sz(0,0, "64", 1, fontsize_64x64);
						break;

					default:
						break;
				}
				ssd1306_refresh();
			
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
			for(i=0;i<sizeof(ssd1306_buffer);i++)
			{
				ssd1306_buffer[i] = rand8();
			}
			ssd1306_refresh();

			/* run conway iterations */
			for(i=0;i<500;i++)
			{
				conway(ssd1306_buffer);
				
				/* refresh */
				ssd1306_refresh();
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
