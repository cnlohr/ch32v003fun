/*
 * Single-File-Header for using I2C OLED
 * 03-29-2023 E. Brombaugh
 */

#ifndef _OLED_H
#define _OLED_H

#include <stdint.h>
#include <string.h>

// I2C clock rate
#define I2C_CLKRATE 100000

// OLED I2C address
#define OLED_ADDR 0x3c

// I2C Timeout count
#define TIMEOUT_MAX 100000

/*
 * init I2C
 */
void i2c_init(void)
{
	uint16_t tempreg;
	
	// Enable GPIOC and I2C
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;
	
	// PC1/PC2 are SDA/SCL, 50MHz Output PP CNF = 11: Mux OD, MODE = 11: Out 50MHz
	GPIOC->CFGLR &= ~(GPIO_CFGLR_MODE1 | GPIO_CFGLR_CNF1 |
						GPIO_CFGLR_MODE1 | GPIO_CFGLR_CNF1);
	GPIOC->CFGLR |= GPIO_CFGLR_CNF1_1 | GPIO_CFGLR_CNF1_0 |
					GPIO_CFGLR_MODE1_1 | GPIO_CFGLR_MODE1_0 |
					GPIO_CFGLR_CNF2_1 | GPIO_CFGLR_CNF2_0 |
					GPIO_CFGLR_MODE2_1 | GPIO_CFGLR_MODE2_0;
		
	// Reset I2C1 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;
	
	// set freq
	tempreg = I2C1->CTLR2;
	tempreg &= ~I2C_CTLR2_FREQ;
	tempreg |= (APB_CLOCK/1000000)&I2C_CTLR2_FREQ;
	I2C1->CTLR2 = tempreg;
	
	// Set clock config
	tempreg = 0;
#if (I2C_CLKRATE <= 100000)
	// standard mode good to 100kHz
	tempreg = (APB_CLOCK/(2*I2C_CLKRATE))&I2C_CKCFGR_CCR;
#else
	// fast mode not yet handled here
#endif
	I2C1->CKCFGR = tempreg;
	
	// Enable I2C
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// set ACK mode
	I2C1->CTLR1 |= I2C_CTLR1_ACK;
	
	// is this needed for a master?
	I2C1->OADDR1 = OLED_ADDR;
}

/*
 * error handler
 */
void i2c_error(void)
{
	// toggle SWRST bit
	I2C1->CTLR1 |= I2C_CTLR1_SWRST;
	I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;	
}

// event codes we use
#define  I2C_EVENT_MASTER_MODE_SELECT ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */

/*
 * check for 32-bit event codes
 */
uint8_t i2c_chk_evt(uint32_t event_mask)
{
	uint32_t status = (I2C1->STAR2<<16) | I2C1->STAR1;
	return (status & event_mask) == event_mask;
}

/*
 * common packet send
 */
void i2c_send(uint8_t *data, uint8_t sz)
{
	int32_t timeout;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for not busy\n\r");
		i2c_error();
		return;
	}

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for master mode\n\r");
		i2c_error();
		return;
	}
	
	// send 7-bit address + write flag
	I2C1->DATAR = OLED_ADDR<<1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for transmit mode\n\r");
		i2c_error();
		return;
	}

	// send data one byte at a time
	while(sz--)
	{
		// wait for TX Empty
		timeout = TIMEOUT_MAX;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
		if(timeout==-1)
		{
			printf("i2c_send :%d - timeout waiting for tx empty\n\r", sz);
			i2c_error();
			return;
		}
		
		// send command
		I2C1->DATAR = *data++;
	}

	// wait for tx complete
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for byte transmitted\n\r");
		i2c_error();
		return;
	}

	// set STOP condition
	I2C1->CTLR1 |= I2C_CTLR1_STOP;	
}

/*
 * send OLED command byte
 */
void oled_cmd(uint8_t cmd)
{
#if 0
	int32_t timeout;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for not busy\n\r");
		i2c_error();
		return;
	}
	
	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for master mode\n\r");
		i2c_error();
		return;
	}
	
	// send 7-bit address + write flag
	I2C1->DATAR = OLED_ADDR << 1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for transmit mode\n\r");
		i2c_error();
		return;
	}

	// wait for TX Empty
	timeout = TIMEOUT_MAX;
	while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for tx empty\n\r");
		i2c_error();
		return;
	}
		
	// send command flag
	I2C1->DATAR = 0;
	
	// wait for TX Empty
	timeout = TIMEOUT_MAX;
	while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for tx empty\n\r");
		i2c_error();
		return;
	}
		
	// send command
	I2C1->DATAR = cmd;
	
	// wait for tx complete
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_cmd - timeout waiting for byte transmitted\n\r");
		i2c_error();
		return;
	}

	// set STOP condition
	I2C1->CTLR1 |= I2C_CTLR1_STOP;
#else
	uint8_t pkt[2];
	
	pkt[0] = 0;
	pkt[1] = cmd;
	i2c_send(pkt, 2);
#endif
}

/*
 * send OLED data packet (up to 32 bytes)
 */
void oled_data(uint8_t *data, uint8_t sz)
{
#if 1
	int32_t timeout;
	
	// max 32 bytes
	sz = sz > 32 ? 32 : sz;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_data - timeout waiting for not busy\n\r");
		i2c_error();
		return;
	}

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_data - timeout waiting for master mode\n\r");
		i2c_error();
		return;
	}
	
	// send 7-bit address + write flag
	I2C1->DATAR = OLED_ADDR<<1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_data - timeout waiting for transmit mode\n\r");
		i2c_error();
		return;
	}

	// wait for TX Empty
	timeout = TIMEOUT_MAX;
	while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_data H - timeout waiting for tx empty\n\r");
		i2c_error();
		return;
	}
		
	// send data flag
	I2C1->DATAR = 0x40;
	
	// send data one byte at a time
	while(sz--)
	{
		// wait for TX Empty
		timeout = TIMEOUT_MAX;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
		if(timeout==-1)
		{
			printf("oled_data P:%d - timeout waiting for tx empty\n\r", sz);
			i2c_error();
			return;
		}
		
		// send command
		I2C1->DATAR = *data++;
	}

	// wait for tx complete
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("oled_data - timeout waiting for byte transmitted\n\r");
		i2c_error();
		return;
	}

	// set STOP condition
	I2C1->CTLR1 |= I2C_CTLR1_STOP;
#else
	uint8_t pkt[33];
	
	// max 32 bytes
	sz = sz > 32 ? 32 : sz;
	
	pkt[0] = 0x40;
	memcpy(&pkt[1], data, sz);
	i2c_send(pkt, sz+1);
#endif
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
#define I2C_PSZ 1
void oled_refresh(void)
{
	uint16_t i;
	
	oled_cmd(SSD1306_COLUMNADDR);
	oled_cmd(0);   // Column start address (0 = reset)
	oled_cmd(OLED_W-1); // Column end address (127 = reset)
	
	oled_cmd(SSD1306_PAGEADDR);
	oled_cmd(0); // Page start address (0 = reset)
	oled_cmd(3); // Page end address

    for (i=0;i<sizeof(oled_buffer);i+=I2C_PSZ)
	{
		/* send a block of data */
		oled_data(&oled_buffer[i], I2C_PSZ);
	}
}

/*
 * initialize act
 */
void oled_init( void )
{
	// initialize port
	i2c_init();
	
	// initialize OLED
	uint8_t *cmd_list = oled_init_array;
	while(*cmd_list != SSD1306_TERMINATE_CMDS)
	{
		oled_cmd(*cmd_list++);
	}
	printf("oled_init - list complete\n\r");
	
	// clear display
	oled_setbuf(0);
	oled_refresh();
}

#endif
