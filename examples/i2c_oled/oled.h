/*
 * Single-File-Header for using I2C OLED
 * 03-29-2023 E. Brombaugh
 */

#ifndef _OLED_H
#define _OLED_H

#include <stdint.h>
#include <string.h>
#include "i2c.h"

// OLED I2C address
#define OLED_ADDR 0x3c

/*
 * send OLED command byte
 */
uint8_t oled_cmd(uint8_t cmd)
{
	uint8_t pkt[2];
	
	pkt[0] = 0;
	pkt[1] = cmd;
	return i2c_send(OLED_ADDR, pkt, 2);
}

/*
 * send OLED data packet (up to 32 bytes)
 */
uint8_t oled_data(uint8_t *data, uint8_t sz)
{
	uint8_t pkt[33];
	
	// max 32 bytes
	sz = sz > 32 ? 32 : sz;
	
	pkt[0] = 0x40;
	memcpy(&pkt[1], data, sz);
	return i2c_send(OLED_ADDR, pkt, sz+1);
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

// OLED initialization commands for 64x32
uint8_t oled_init_array[] =
{
    SSD1306_DISPLAYOFF,                    // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,            // 0xD5
    0x80,                                  // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX,                  // 0xA8
    0x1F,                                  // different for tiny
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
    0x12,
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
#define OLED_W 64
#define OLED_H 32

// the display buffer
uint8_t oled_buffer[128*32/8];

/*
 * set the buffer to a color
 */
void oled_setbuf(uint8_t color)
{
	memset(oled_buffer, color ? 0xFF : 0x00, sizeof(oled_buffer));
}

/*
 * Send the frame buffer
 */
#define I2C_PSZ 16
void oled_refresh(void)
{
	uint16_t i;
	
	oled_cmd(SSD1306_COLUMNADDR);
	oled_cmd(0);   // Column start address (0 = reset)
	oled_cmd(OLED_W-1); // Column end address (127 = reset)
	
	oled_cmd(SSD1306_PAGEADDR);
	oled_cmd(0); // Page start address (0 = reset)
	oled_cmd(3); // Page end address

    for(i=0;i<sizeof(oled_buffer);i+=I2C_PSZ)
	{
		/* send a block of data */
		oled_data(&oled_buffer[i], I2C_PSZ);
	}
}

/*
 * initialize act
 */
uint8_t oled_init( void )
{
	// initialize port
	i2c_init();
	
	// initialize OLED
	uint8_t *cmd_list = oled_init_array;
	while(*cmd_list != SSD1306_TERMINATE_CMDS)
	{
		if(oled_cmd(*cmd_list++))
			return 1;
	}
	
	// clear display
	oled_setbuf(0);
	oled_refresh();
	
	return 0;
}

#endif
