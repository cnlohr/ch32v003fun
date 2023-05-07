/*
 * Single-File-Header for using SPI OLED
 * 05-05-2023 E. Brombaugh
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#include <stdint.h>
#include <string.h>
#include "font_8x8.h"

// comfortable packet size for this OLED
#define SSD1306_PSZ 32

// characteristics of each type
#if !defined (SSD1306_64X32) && !defined (SSD1306_128X32) && !defined (SSD1306_128X64)
	#error "Please define the SSD1306_WXH resolution used in your application"
#endif

#ifdef SSD1306_64X32
#define SSD1306_W 64
#define SSD1306_H 32
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 32
#endif

#ifdef SSD1306_128X32
#define SSD1306_W 128
#define SSD1306_H 32
#define SSD1306_OFFSET 0
#endif

#ifdef SSD1306_128X64
#define SSD1306_W 128
#define SSD1306_H 64
#define SSD1306_FULLUSE
#define SSD1306_OFFSET 0
#endif

/*
 * send OLED command byte
 */
uint8_t ssd1306_cmd(uint8_t cmd)
{
	ssd1306_pkt_send(&cmd, 1, 1);
	return 0;
}

/*
 * send OLED data packet (up to 32 bytes)
 */
uint8_t ssd1306_data(uint8_t *data, uint8_t sz)
{
	ssd1306_pkt_send(data, sz, 0);
	return 0;
}

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_TERMINATE_CMDS 0xFF

/* choose VCC mode */
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
//#define vccstate SSD1306_EXTERNALVCC
#define vccstate SSD1306_SWITCHCAPVCC

// OLED initialization commands for 128x32
const uint8_t ssd1306_init_array[] =
{
    SSD1306_DISPLAYOFF,                    // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
    0x80,                                  // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX,                  // 0xA8
#ifdef SSD1306_64X32
	0x1F,                                  // for 64-wide displays
#else
	0x3F,                                  // for 128-wide displays
#endif
    SSD1306_SETDISPLAYOFFSET,              // 0xD3
    0x00,                                  // no offset
	SSD1306_SETSTARTLINE | 0x0,            // 0x40 | line
    SSD1306_CHARGEPUMP,                    // 0x8D
	0x14,                                  // enable?
    SSD1306_MEMORYMODE,                    // 0x20
    0x00,                                  // 0x0 act like ks0108
    SSD1306_SEGREMAP | 0x1,                // 0xA0 | bit
    SSD1306_COMSCANDEC,
    SSD1306_SETCOMPINS,                    // 0xDA
	0x12,                                  //
    SSD1306_SETCONTRAST,                   // 0x81
	0x8F,
    SSD1306_SETPRECHARGE,                  // 0xd9
	0xF1,
    SSD1306_SETVCOMDETECT,                 // 0xDB
    0x40,
    SSD1306_DISPLAYALLON_RESUME,           // 0xA4
    SSD1306_NORMALDISPLAY,                 // 0xA6
	SSD1306_DISPLAYON,	                   // 0xAF --turn on oled panel
	SSD1306_TERMINATE_CMDS                 // 0xFF --fake command to mark end
};

// the display buffer
uint8_t ssd1306_buffer[SSD1306_W*SSD1306_H/8];

/*
 * set the buffer to a color
 */
void ssd1306_setbuf(uint8_t color)
{
	memset(ssd1306_buffer, color ? 0xFF : 0x00, sizeof(ssd1306_buffer));
}

#ifndef SSD1306_FULLUSE
/*
 * expansion array for OLED with every other row unused
 */
const uint8_t expand[16] =
{
	0x00,0x02,0x08,0x0a,
	0x20,0x22,0x28,0x2a,
	0x80,0x82,0x88,0x8a,
	0xa0,0xa2,0xa8,0xaa,
};
#endif

/*
 * Send the frame buffer
 */
void ssd1306_refresh(void)
{
	uint16_t i;
	
	ssd1306_cmd(SSD1306_COLUMNADDR);
	ssd1306_cmd(SSD1306_OFFSET);   // Column start address (0 = reset)
	ssd1306_cmd(SSD1306_OFFSET+SSD1306_W-1); // Column end address (127 = reset)
	
	ssd1306_cmd(SSD1306_PAGEADDR);
	ssd1306_cmd(0); // Page start address (0 = reset)
	ssd1306_cmd(7); // Page end address

#ifdef SSD1306_FULLUSE
	/* for fully used rows just plow thru everything */
    for(i=0;i<sizeof(ssd1306_buffer);i+=SSD1306_PSZ)
	{
		/* send PSZ block of data */
		ssd1306_data(&ssd1306_buffer[i], SSD1306_PSZ);
	}
#else
	/* for displays with odd rows unused expand bytes */
	uint8_t tbuf[SSD1306_PSZ], j, k;
    for(i=0;i<sizeof(ssd1306_buffer);i+=128)
	{
		/* low nybble */
		for(j=0;j<128;j+=SSD1306_PSZ)
		{
			for(k=0;k<SSD1306_PSZ;k++)
				tbuf[k] = expand[ssd1306_buffer[i+j+k]&0xf];
			
			/* send PSZ block of data */
			ssd1306_data(tbuf, SSD1306_PSZ);
		}
		
		/* high nybble */
		for(j=0;j<128;j+=SSD1306_PSZ)
		{
			for(k=0;k<SSD1306_PSZ;k++)
				tbuf[k] = expand[(ssd1306_buffer[i+j+k]>>4)&0xf];
			
			/* send PSZ block of data */
			ssd1306_data(tbuf, SSD1306_PSZ);
		}
	}
#endif
}

/*
 * plot a pixel in the buffer
 */
void ssd1306_drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint16_t addr;
	
	/* clip */
	if(x >= SSD1306_W)
		return;
	if(y >= SSD1306_H)
		return;
	
	/* compute buffer address */
	addr = x + SSD1306_W*(y/8);
	
	/* set/clear bit in buffer */
	if(color)
		ssd1306_buffer[addr] |= (1<<(y&7));
	else
		ssd1306_buffer[addr] &= ~(1<<(y&7));
}

/*
 * plot a pixel in the buffer
 */
void ssd1306_xorPixel(uint8_t x, uint8_t y)
{
	uint16_t addr;
	
	/* clip */
	if(x >= SSD1306_W)
		return;
	if(y >= SSD1306_H)
		return;
	
	/* compute buffer address */
	addr = x + SSD1306_W*(y/8);
	
	ssd1306_buffer[addr] ^= (1<<(y&7));
}

/*
 *  fast vert line
 */
void ssd1306_drawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color)
{
	// clipping
	if((x >= SSD1306_W) || (y >= SSD1306_H)) return;
	if((y+h-1) >= SSD1306_H) h = SSD1306_H-y;
	while(h--)
	{
        ssd1306_drawPixel(x, y++, color);
	}
}

/*
 *  fast horiz line
 */
void ssd1306_drawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
	// clipping
	if((x >= SSD1306_W) || (y >= SSD1306_H)) return;
	if((x+w-1) >= SSD1306_W)  w = SSD1306_W-x;

	while (w--)
	{
        ssd1306_drawPixel(x++, y, color);
	}
}

/*
 * abs() helper function for line drawing
 */
int16_t gfx_abs(int16_t x)
{
	return (x<0) ? -x : x;
}

/*
 * swap() helper function for line drawing
 */
void gfx_swap(uint16_t *z0, uint16_t *z1)
{
	uint16_t temp = *z0;
	*z0 = *z1;
	*z1 = temp;
}

/*
 * Bresenham line draw routine swiped from Wikipedia
 */
void ssd1306_drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
	int16_t steep;
	int16_t deltax, deltay, error, ystep, x, y;

	/* flip sense 45deg to keep error calc in range */
	steep = (gfx_abs(y1 - y0) > gfx_abs(x1 - x0));

	if(steep)
	{
		gfx_swap(&x0, &y0);
		gfx_swap(&x1, &y1);
	}

	/* run low->high */
	if(x0 > x1)
	{
		gfx_swap(&x0, &x1);
		gfx_swap(&y0, &y1);
	}

	/* set up loop initial conditions */
	deltax = x1 - x0;
	deltay = gfx_abs(y1 - y0);
	error = deltax/2;
	y = y0;
	if(y0 < y1)
		ystep = 1;
	else
		ystep = -1;

	/* loop x */
	for(x=x0;x<=x1;x++)
	{
		/* plot point */
		if(steep)
			/* flip point & plot */
			ssd1306_drawPixel(y, x, color);
		else
			/* just plot */
			ssd1306_drawPixel(x, y, color);

		/* update error */
		error = error - deltay;

		/* update y */
		if(error < 0)
		{
			y = y + ystep;
			error = error + deltax;
		}
	}
}

/*
 *  draws a circle
 */
void ssd1306_drawCircle(int16_t x, int16_t y, int16_t radius, int8_t color)
{
    /* Bresenham algorithm */
    int16_t x_pos = -radius;
    int16_t y_pos = 0;
    int16_t err = 2 - 2 * radius;
    int16_t e2;

    do {
        ssd1306_drawPixel(x - x_pos, y + y_pos, color);
        ssd1306_drawPixel(x + x_pos, y + y_pos, color);
        ssd1306_drawPixel(x + x_pos, y - y_pos, color);
        ssd1306_drawPixel(x - x_pos, y - y_pos, color);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
              e2 = 0;
            }
        }
        if (e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while (x_pos <= 0);
}

/*
 *  draws a filled circle
 */
void ssd1306_fillCircle(int16_t x, int16_t y, int16_t radius, int8_t color)
{
    /* Bresenham algorithm */
    int16_t x_pos = -radius;
    int16_t y_pos = 0;
    int16_t err = 2 - 2 * radius;
    int16_t e2;

    do {
        ssd1306_drawPixel(x - x_pos, y + y_pos, color);
        ssd1306_drawPixel(x + x_pos, y + y_pos, color);
        ssd1306_drawPixel(x + x_pos, y - y_pos, color);
        ssd1306_drawPixel(x - x_pos, y - y_pos, color);
        ssd1306_drawFastHLine(x + x_pos, y + y_pos, 2 * (-x_pos) + 1, color);
        ssd1306_drawFastHLine(x + x_pos, y - y_pos, 2 * (-x_pos) + 1, color);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
                e2 = 0;
            }
        }
        if(e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while(x_pos <= 0);
}

/*
 *  draw a rectangle
 */
void ssd1306_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	ssd1306_drawFastVLine(x, y, h, color);
	ssd1306_drawFastVLine(x+w-1, y, h, color);
	ssd1306_drawFastHLine(x, y, w, color);
	ssd1306_drawFastHLine(x, y+h-1, w, color);
}

/*
 * fill a rectangle
 */
void ssd1306_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	uint8_t m, n=y, iw = w;
	
	/* scan vertical */
	while(h--)
	{
		m=x;
		w=iw;
		/* scan horizontal */
		while(w--)
		{
			/* invert pixels */
			ssd1306_drawPixel(m++, n, color);
		}
		n++;
	}
}

/*
 * invert a rectangle in the buffer
 */
void ssd1306_xorrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t m, n=y, iw = w;
	
	/* scan vertical */
	while(h--)
	{
		m=x;
		w=iw;
		/* scan horizontal */
		while(w--)
		{
			/* invert pixels */
			ssd1306_xorPixel(m++, n);
		}
		n++;
	}
}

/*
 * Draw character to the display buffer
 */
void ssd1306_drawchar(uint8_t x, uint8_t y, uint8_t chr, uint8_t color)
{
	uint16_t i, j, col;
	uint8_t d;
	
	for(i=0;i<8;i++)
	{
		d = fontdata[(chr<<3)+i];
		for(j=0;j<8;j++)
		{
			if(d&0x80)
				col = color;
			else
				col = (~color)&1;
			
			ssd1306_drawPixel(x+j, y+i, col);
			
			// next bit
			d <<= 1;
		}
	}
}

/*
 * draw a string to the display
 */
void ssd1306_drawstr(uint8_t x, uint8_t y, char *str, uint8_t color)
{
	uint8_t c;
	
	while((c=*str++))
	{
		ssd1306_drawchar(x, y, c, color);
		x += 8;
		if(x>120)
			break;
	}
}

/*
 * enum for font size
 */
typedef enum {
    fontsize_8x8 = 1,
    fontsize_16x16 = 2,
    fontsize_32x32 = 4,
	fontsize_64x64 = 8,
} font_size_t;

/*
 * Draw character to the display buffer, scaled to size
 */
void ssd1306_drawchar_sz(uint8_t x, uint8_t y, uint8_t chr, uint8_t color, font_size_t font_size)
{
    uint16_t i, j, col;
    uint8_t d;

    // Determine the font scale factor based on the font_size parameter
    uint8_t font_scale = (uint8_t)font_size;

    // Loop through each row of the font data
    for (i = 0; i < 8; i++)
    {
        // Retrieve the font data for the current row
        d = fontdata[(chr << 3) + i];

        // Loop through each column of the font data
        for (j = 0; j < 8; j++)
        {
            // Determine the color to draw based on the current bit in the font data
            if (d & 0x80)
                col = color;
            else
                col = (~color) & 1;

            // Draw the pixel at the original size and scaled size using nested for-loops
            for (uint8_t k = 0; k < font_scale; k++) {
                for (uint8_t l = 0; l < font_scale; l++) {
                    ssd1306_drawPixel(x + (j * font_scale) + k, y + (i * font_scale) + l, col);
                }
            }

            // Move to the next bit in the font data
            d <<= 1;
        }
    }
}

/*
 * draw a string to the display buffer, scaled to size
 */
void ssd1306_drawstr_sz(uint8_t x, uint8_t y, char *str, uint8_t color, font_size_t font_size)
{
	uint8_t c;
	
	while((c=*str++))
	{
		ssd1306_drawchar_sz(x, y, c, color, font_size);
		x += 8 * font_size;
		if(x>128 - 8 * font_size)
			break;
	}
}

/*
 * initialize I2C and OLED
 */
uint8_t ssd1306_init(void)
{
	// pulse reset
	ssd1306_rst();
	
	// initialize OLED
	uint8_t *cmd_list = (uint8_t *)ssd1306_init_array;
	while(*cmd_list != SSD1306_TERMINATE_CMDS)
	{
		if(ssd1306_cmd(*cmd_list++))
			return 1;
	}
	
	// clear display
	ssd1306_setbuf(0);
	ssd1306_refresh();
	
	return 0;
}

#endif
