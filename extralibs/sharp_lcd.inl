/*
 * sharp_lcd.c
 *
 *  Created on: Jan 27, 2023
 *      Author: Larry Bank
 */
#include <stdint.h>
#include <string.h>
#include "sharp_lcd.h"

static uint8_t u8CSPin;
static uint8_t u8Cache[(LCD_PITCH * LCD_HEIGHT)+2];
static uint8_t cursor_x, cursor_y;
volatile int bDMA = 0;

// 7x7 font (in 8x8 cell)
const uint8_t ucFont[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x5f,0x5f,0x06,0x00,
  0x00,0x07,0x07,0x00,0x07,0x07,0x00,0x14,0x7f,0x7f,0x14,0x7f,0x7f,0x14,
  0x24,0x2e,0x2a,0x6b,0x6b,0x3a,0x12,0x46,0x66,0x30,0x18,0x0c,0x66,0x62,
  0x30,0x7a,0x4f,0x5d,0x37,0x7a,0x48,0x00,0x04,0x07,0x03,0x00,0x00,0x00,
  0x00,0x1c,0x3e,0x63,0x41,0x00,0x00,0x00,0x41,0x63,0x3e,0x1c,0x00,0x00,
  0x08,0x2a,0x3e,0x1c,0x3e,0x2a,0x08,0x00,0x08,0x08,0x3e,0x3e,0x08,0x08,
  0x00,0x00,0x80,0xe0,0x60,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,
  0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x30,0x18,0x0c,0x06,0x03,0x01,
  0x3e,0x7f,0x59,0x4d,0x47,0x7f,0x3e,0x40,0x42,0x7f,0x7f,0x40,0x40,0x00,
  0x62,0x73,0x59,0x49,0x6f,0x66,0x00,0x22,0x63,0x49,0x49,0x7f,0x36,0x00,
  0x18,0x1c,0x16,0x53,0x7f,0x7f,0x50,0x27,0x67,0x45,0x45,0x7d,0x39,0x00,
  0x3c,0x7e,0x4b,0x49,0x79,0x30,0x00,0x03,0x03,0x71,0x79,0x0f,0x07,0x00,
  0x36,0x7f,0x49,0x49,0x7f,0x36,0x00,0x06,0x4f,0x49,0x69,0x3f,0x1e,0x00,
  0x00,0x00,0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x80,0xe6,0x66,0x00,0x00,
  0x08,0x1c,0x36,0x63,0x41,0x00,0x00,0x00,0x14,0x14,0x14,0x14,0x14,0x14,
  0x00,0x41,0x63,0x36,0x1c,0x08,0x00,0x00,0x02,0x03,0x59,0x5d,0x07,0x02,
  0x3e,0x7f,0x41,0x5d,0x5d,0x5f,0x0e,0x7c,0x7e,0x13,0x13,0x7e,0x7c,0x00,
  0x41,0x7f,0x7f,0x49,0x49,0x7f,0x36,0x1c,0x3e,0x63,0x41,0x41,0x63,0x22,
  0x41,0x7f,0x7f,0x41,0x63,0x3e,0x1c,0x41,0x7f,0x7f,0x49,0x5d,0x41,0x63,
  0x41,0x7f,0x7f,0x49,0x1d,0x01,0x03,0x1c,0x3e,0x63,0x41,0x51,0x33,0x72,
  0x7f,0x7f,0x08,0x08,0x7f,0x7f,0x00,0x00,0x41,0x7f,0x7f,0x41,0x00,0x00,
  0x30,0x70,0x40,0x41,0x7f,0x3f,0x01,0x41,0x7f,0x7f,0x08,0x1c,0x77,0x63,
  0x41,0x7f,0x7f,0x41,0x40,0x60,0x70,0x7f,0x7f,0x0e,0x1c,0x0e,0x7f,0x7f,
  0x7f,0x7f,0x06,0x0c,0x18,0x7f,0x7f,0x1c,0x3e,0x63,0x41,0x63,0x3e,0x1c,
  0x41,0x7f,0x7f,0x49,0x09,0x0f,0x06,0x1e,0x3f,0x21,0x31,0x61,0x7f,0x5e,
  0x41,0x7f,0x7f,0x09,0x19,0x7f,0x66,0x26,0x6f,0x4d,0x49,0x59,0x73,0x32,
  0x03,0x41,0x7f,0x7f,0x41,0x03,0x00,0x7f,0x7f,0x40,0x40,0x7f,0x7f,0x00,
  0x1f,0x3f,0x60,0x60,0x3f,0x1f,0x00,0x3f,0x7f,0x60,0x30,0x60,0x7f,0x3f,
  0x63,0x77,0x1c,0x08,0x1c,0x77,0x63,0x07,0x4f,0x78,0x78,0x4f,0x07,0x00,
  0x47,0x63,0x71,0x59,0x4d,0x67,0x73,0x00,0x7f,0x7f,0x41,0x41,0x00,0x00,
  0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0x00,0x41,0x41,0x7f,0x7f,0x00,0x00,
  0x08,0x0c,0x06,0x03,0x06,0x0c,0x08,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
  0x00,0x00,0x03,0x07,0x04,0x00,0x00,0x20,0x74,0x54,0x54,0x3c,0x78,0x40,
  0x41,0x7f,0x3f,0x48,0x48,0x78,0x30,0x38,0x7c,0x44,0x44,0x6c,0x28,0x00,
  0x30,0x78,0x48,0x49,0x3f,0x7f,0x40,0x38,0x7c,0x54,0x54,0x5c,0x18,0x00,
  0x48,0x7e,0x7f,0x49,0x03,0x06,0x00,0x98,0xbc,0xa4,0xa4,0xf8,0x7c,0x04,
  0x41,0x7f,0x7f,0x08,0x04,0x7c,0x78,0x00,0x44,0x7d,0x7d,0x40,0x00,0x00,
  0x60,0xe0,0x80,0x84,0xfd,0x7d,0x00,0x41,0x7f,0x7f,0x10,0x38,0x6c,0x44,
  0x00,0x41,0x7f,0x7f,0x40,0x00,0x00,0x7c,0x7c,0x18,0x78,0x1c,0x7c,0x78,
  0x7c,0x78,0x04,0x04,0x7c,0x78,0x00,0x38,0x7c,0x44,0x44,0x7c,0x38,0x00,
  0x84,0xfc,0xf8,0xa4,0x24,0x3c,0x18,0x18,0x3c,0x24,0xa4,0xf8,0xfc,0x84,
  0x44,0x7c,0x78,0x4c,0x04,0x0c,0x18,0x48,0x5c,0x54,0x74,0x64,0x24,0x00,
  0x04,0x04,0x3e,0x7f,0x44,0x24,0x00,0x3c,0x7c,0x40,0x40,0x3c,0x7c,0x40,
  0x1c,0x3c,0x60,0x60,0x3c,0x1c,0x00,0x3c,0x7c,0x60,0x30,0x60,0x7c,0x3c,
  0x44,0x6c,0x38,0x10,0x38,0x6c,0x44,0x9c,0xbc,0xa0,0xa0,0xfc,0x7c,0x00,
  0x4c,0x64,0x74,0x5c,0x4c,0x64,0x00,0x08,0x08,0x3e,0x77,0x41,0x41,0x00,
  0x00,0x00,0x00,0x77,0x77,0x00,0x00,0x41,0x41,0x77,0x3e,0x08,0x08,0x00,
  0x02,0x03,0x01,0x03,0x02,0x03,0x01,0x70,0x78,0x4c,0x46,0x4c,0x78,0x70};
// 5x7 font (in 6x8 cell)
const uint8_t ucSmallFont[] = {
0x00,0x00,0x00,0x00,0x00,
0x00,0x06,0x5f,0x06,0x00,
0x07,0x03,0x00,0x07,0x03,
0x24,0x7e,0x24,0x7e,0x24,
0x24,0x2b,0x6a,0x12,0x00,
0x63,0x13,0x08,0x64,0x63,
0x36,0x49,0x56,0x20,0x50,
0x00,0x07,0x03,0x00,0x00,
0x00,0x3e,0x41,0x00,0x00,
0x00,0x41,0x3e,0x00,0x00,
0x08,0x3e,0x1c,0x3e,0x08,
0x08,0x08,0x3e,0x08,0x08,
0x00,0xe0,0x60,0x00,0x00,
0x08,0x08,0x08,0x08,0x08,
0x00,0x60,0x60,0x00,0x00,
0x20,0x10,0x08,0x04,0x02,
0x3e,0x51,0x49,0x45,0x3e,
0x00,0x42,0x7f,0x40,0x00,
0x62,0x51,0x49,0x49,0x46,
0x22,0x49,0x49,0x49,0x36,
0x18,0x14,0x12,0x7f,0x10,
0x2f,0x49,0x49,0x49,0x31,
0x3c,0x4a,0x49,0x49,0x30,
0x01,0x71,0x09,0x05,0x03,
0x36,0x49,0x49,0x49,0x36,
0x06,0x49,0x49,0x29,0x1e,
0x00,0x6c,0x6c,0x00,0x00,
0x00,0xec,0x6c,0x00,0x00,
0x08,0x14,0x22,0x41,0x00,
0x24,0x24,0x24,0x24,0x24,
0x00,0x41,0x22,0x14,0x08,
0x02,0x01,0x59,0x09,0x06,
0x3e,0x41,0x5d,0x55,0x1e,
0x7e,0x11,0x11,0x11,0x7e,
0x7f,0x49,0x49,0x49,0x36,
0x3e,0x41,0x41,0x41,0x22,
0x7f,0x41,0x41,0x41,0x3e,
0x7f,0x49,0x49,0x49,0x41,
0x7f,0x09,0x09,0x09,0x01,
0x3e,0x41,0x49,0x49,0x7a,
0x7f,0x08,0x08,0x08,0x7f,
0x00,0x41,0x7f,0x41,0x00,
0x30,0x40,0x40,0x40,0x3f,
0x7f,0x08,0x14,0x22,0x41,
0x7f,0x40,0x40,0x40,0x40,
0x7f,0x02,0x04,0x02,0x7f,
0x7f,0x02,0x04,0x08,0x7f,
0x3e,0x41,0x41,0x41,0x3e,
0x7f,0x09,0x09,0x09,0x06,
0x3e,0x41,0x51,0x21,0x5e,
0x7f,0x09,0x09,0x19,0x66,
0x26,0x49,0x49,0x49,0x32,
0x01,0x01,0x7f,0x01,0x01,
0x3f,0x40,0x40,0x40,0x3f,
0x1f,0x20,0x40,0x20,0x1f,
0x3f,0x40,0x3c,0x40,0x3f,
0x63,0x14,0x08,0x14,0x63,
0x07,0x08,0x70,0x08,0x07,
0x71,0x49,0x45,0x43,0x00,
0x00,0x7f,0x41,0x41,0x00,
0x02,0x04,0x08,0x10,0x20,
0x00,0x41,0x41,0x7f,0x00,
0x04,0x02,0x01,0x02,0x04,
0x80,0x80,0x80,0x80,0x80,
0x00,0x03,0x07,0x00,0x00,
0x20,0x54,0x54,0x54,0x78,
0x7f,0x44,0x44,0x44,0x38,
0x38,0x44,0x44,0x44,0x28,
0x38,0x44,0x44,0x44,0x7f,
0x38,0x54,0x54,0x54,0x08,
0x08,0x7e,0x09,0x09,0x00,
0x18,0xa4,0xa4,0xa4,0x7c,
0x7f,0x04,0x04,0x78,0x00,
0x00,0x00,0x7d,0x40,0x00,
0x40,0x80,0x84,0x7d,0x00,
0x7f,0x10,0x28,0x44,0x00,
0x00,0x00,0x7f,0x40,0x00,
0x7c,0x04,0x18,0x04,0x78,
0x7c,0x04,0x04,0x78,0x00,
0x38,0x44,0x44,0x44,0x38,
0xfc,0x44,0x44,0x44,0x38,
0x38,0x44,0x44,0x44,0xfc,
0x44,0x78,0x44,0x04,0x08,
0x08,0x54,0x54,0x54,0x20,
0x04,0x3e,0x44,0x24,0x00,
0x3c,0x40,0x20,0x7c,0x00,
0x1c,0x20,0x40,0x20,0x1c,
0x3c,0x60,0x30,0x60,0x3c,
0x6c,0x10,0x10,0x6c,0x00,
0x9c,0xa0,0x60,0x3c,0x00,
0x64,0x54,0x54,0x4c,0x00,
0x08,0x3e,0x41,0x41,0x00,
0x00,0x00,0x77,0x00,0x00,
0x00,0x41,0x41,0x3e,0x08,
0x02,0x01,0x02,0x01,0x00,
0x3c,0x26,0x23,0x26,0x3c};

#ifdef FUTURE
void DMA1_Channel3_IRQHandler(void) __attribute__((interrupt));

void DMA1_Channel3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC3)) {
		DMA_ClearITPendingBit(DMA1_IT_TC3);
		bDMA = 0; // no longer active transaction
		DMA_Cmd(DMA1_Channel3, DISABLE);
		digitalWrite(u8CSPin, 0); // de-activate CS
	}
	// clear all other flags
	DMA1->INTFCR = DMA1_IT_GL3;
}
#endif

void sharpWriteBuffer(void)
{
	// use polling SPI
   digitalWrite(u8CSPin, 1); // activate CS
   SPI_write(u8Cache, sizeof(u8Cache)); // write it all in once shot
   digitalWrite(u8CSPin, 0);
#ifdef FUTURE
	while (bDMA) {}; // wait for old transaction to complete
    DMA1_Channel3->CNTR = (LCD_PITCH * LCD_HEIGHT)+2;
    DMA1_Channel3->MADDR = (uint32_t)u8Cache;
	digitalWrite(u8CSPin, 1); // activate CS
	DMA_Cmd(DMA1_Channel3, ENABLE); // have DMA send the data
	bDMA = 1; // tell our code that DMA is currently active for next time
#endif
} /* sharpWriteBuffer() */


void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
#ifdef FUTURE
    DMA_InitTypeDef DMA_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure={0};

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // Enable DMA interrupt on channel 3
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ( DMA1_Channel3_IRQn );

    DMA_Cmd(DMA1_Channel3, DISABLE);

    DMA_DeInit(DMA_CHx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
#endif // FUTURE
} /* DMA_Tx_Init() */

// Less efficient than a lookup table, but it's only used at init time
// This saves a couple hundred bytes of FLASH
uint8_t MirrorBits(uint8_t v)
{
	uint8_t r = v & 1;
	uint8_t s = 7;
	for (v >>= 1; v; v >>= 1) {
		r <<= 1;
		r |= (v & 1);
		s--;
	}
	r <<= s; // adjust for 0's
	return r;
} /* MirrorBits() */

void sharpInit(int iSpeed, uint8_t u8CS)
{
	int i;
	uint8_t *d;
   u8CSPin = u8CS;
   SPI_begin(iSpeed, 0);
   pinMode(u8CSPin, OUTPUT);
   // set up memory buffer so that every line can be dumped in a single DMA transaction
   u8Cache[0] = 0x80; // start byte
   d = &u8Cache[1];
   for (i=0; i<LCD_HEIGHT; i++) { // pre-fill line numbers and stop bytes
	   d[0] = MirrorBits(i+1); // line number
	   // bytes 1-20 hold the 160 pixels for this line
	   d[LCD_PITCH-1] = 0; // stop byte
	   d += LCD_PITCH;
   }
   u8Cache[(LCD_HEIGHT*LCD_PITCH)+1] = 0; // final double-stop byte

   DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DATAR, (u32)u8Cache, 0);
} /* sharpInit() */

uint8_t * sharpGetBuffer(void)
{
	return &u8Cache[2]; // skip start byte and first line number
} /* sharpGetBuffer() */

void sharpVLine(int x, int y1, int y2, int color)
{
uint8_t *d, ucMask;
int i;

	if (y2 < y1) {
		i = y1;
		y1 = y2;
		y2 = i;
	}
	ucMask = (1 << (x & 7));
	d = sharpGetBuffer();
	d += (x >> 3) + (y1 * LCD_PITCH);
	if (color) {
		for (i=y1; i<=y2; i++) {
			d[0] |= ucMask;
			d += LCD_PITCH;
		}
	} else {
		for (i=y1; i<=y2; i++) {
			d[0] &= ~ucMask;
			d += LCD_PITCH;
		}
	}
} /* sharpVLine() */

void sharpHLine(int x1, int x2, int y, int color)
{
uint8_t *d, ucL, ucR;
int i, len;

	if (x2 < x1) {
		i = x1;
		x1 = x2;
		x2 = i;
	}
	len = (x2 >> 3) - (x1 >> 3);
	d = sharpGetBuffer();
	d += (x1 >> 3) + (y * LCD_PITCH);
	if (color) {
		ucL = 0xff << (x1 & 7);
		ucR = 0xff >> (7-(x2 & 7));
		if (len == 0) {
			ucL &= ucR;
			d[0] |= ucL;
		} else {
			*d |= ucL;
			d++;
			while (len > 1) {
				*d++ = 0xff;
				len--;
			}
			*d |= ucR;
		}
	} else {
		ucL = 0xff << (8-(x1 & 7));
		ucR = 0xff >> (x2 & 7);
		if (len == 0) {
			ucL |= ucR;
			d[0] &= ucL;
		} else {
			*d++ &= ucL;
			while (len > 1) {
				*d++ = 0;
				len--;
			}
			*d &= ucR;
		}
	}
} /* sharpHLine() */

void sharpFill(uint8_t u8Pattern)
{
	uint8_t *d;
	int i;
	while (bDMA) {}; // can't change the buffer while DMA is active because we only have 1!!!
	d = sharpGetBuffer();
	for (i=0; i<LCD_HEIGHT; i++) {
		memset(d, u8Pattern, (LCD_WIDTH>>3));
		d += LCD_PITCH;
	}
} /* sharpFill() */

void sharpInvert(void)
{
	uint8_t *d;
	int i, j;

	while (bDMA) {}; // can't change the buffer while DMA is active because we only have 1!!!
	d = sharpGetBuffer();
	for (i=0; i<LCD_HEIGHT; i++) {
		for (j=0; j<(LCD_WIDTH>>3); j++) {
			d[j] = ~d[j];
		} // for j
		d += LCD_PITCH;
	} // for i
} /* sharpInvert() */
void sharpDrawSprite(int x, int y, int cx, int cy, uint8_t *pSprite, int iPitch, int bInvert)
{
    int tx, ty, dx, dy, iStartX;
    uint8_t *pBuf, *s, *d, pix, ucSrcMask, ucDstMask, u8Invert;

    pBuf = sharpGetBuffer();
    u8Invert = (bInvert) ? 0xff : 0x00;
    if (x+cx < 0 || y+cy < 0 || x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return; // out of bounds
    dy = y; // destination y
    if (y < 0) // skip the invisible parts
    {
        cy += y;
        y = -y;
        pSprite += (y * iPitch);
        dy = 0;
    }
    if (y + cy > LCD_HEIGHT)
        cy = LCD_HEIGHT - y;
    dx = x;
    iStartX = 0;
    if (x < 0)
    {
        cx += x;
        x = -x;
        iStartX = x;
        dx = 0;
    }
    if (x + cx > LCD_WIDTH)
        cx = LCD_WIDTH - x;
    for (ty=dy; ty<(dy+cy); ty++)
    {
        s = &pSprite[(iStartX >> 3)];
        d = &pBuf[(ty*LCD_PITCH) + (dx >> 3)];
        ucSrcMask = 0x80 >> (iStartX & 7);
        pix = *s++;
        pix ^= u8Invert;
        ucDstMask = 0x80 >> (dx & 7);
		  for (tx=dx; tx<(dx+cx); tx++)
		  {
			if (pix & ucSrcMask) // set pixel in source, set it in dest
				d[0] &= ~ucDstMask;
			else
				d[0] |= ucDstMask;
			ucDstMask >>= 1;
			if (ucDstMask == 0) { // start next byte
				d++;
				ucDstMask = 0x80;
			}
			ucSrcMask >>= 1;
			if (ucSrcMask == 0) // read next byte
			{
				ucSrcMask = 0x80;
				pix = *s++;
				pix ^= u8Invert;
			}
		  } // for tx
        pSprite += iPitch;
    } // for ty
} /* sharpDrawSprite() */

//
// Draw a string of characters in a custom font
// A back buffer must be defined
//
void sharpWriteStringCustom(const GFXfont *pFont, int x, int y, char *szMsg, uint8_t u8Color, int bFill)
{
	int i, end_y, dx, dy, tx, ty, iBitOff;
	unsigned int c;
	uint8_t *s, *d, bits, ucMask, uc;
	GFXfont font;
	GFXglyph glyph, *pGlyph;

	    if (x == -1)
	        x = cursor_x;
	    if (y == -1)
	        y = cursor_y;
	   // in case of running on Harvard architecture, get copy of data from FLASH
	   memcpy(&font, pFont, sizeof(font));
	   pGlyph = &glyph;

	   i = 0;
	   while (szMsg[i] && x < LCD_WIDTH)
	   {
	      c = szMsg[i++];
	      if (c < font.first || c > font.last) // undefined character
	         continue; // skip it
	      c -= font.first; // first char of font defined
	      memcpy(&glyph, &font.glyph[c], sizeof(glyph));
	      dx = x + pGlyph->xOffset; // offset from character UL to start drawing
	      dy = y + pGlyph->yOffset;
	      s = font.bitmap + pGlyph->bitmapOffset; // start of bitmap data
	      // Bitmap drawing loop. Image is MSB first and each pixel is packed next
	      // to the next (continuing on to the next character line)
	      iBitOff = 0; // bitmap offset (in bits)
	      bits = uc = 0; // bits left in this font byte
	      end_y = dy + pGlyph->height;
	      if (dy < 0) { // skip these lines
	          iBitOff += (pGlyph->width * (-dy));
	          dy = 0;
	      }
	      for (ty=dy; ty<end_y && ty < LCD_HEIGHT; ty++) {
	    	  if (bFill) {
				  // clear the empty part of the character rectangle (left)
	    		  d = sharpGetBuffer();
				  d += (x/8) + (ty * LCD_PITCH);
				  ucMask = 0x80>>(x & 7);
				  for (tx=x; tx<dx; tx++) {
					  if (u8Color == 1)
						 d[0] &= ~ucMask;
					  else
						 d[0] |= ucMask;
					  ucMask >>= 1;
					  if (ucMask == 0) {
						  d++;
						  ucMask = 0x80;
					  }
				  } // for tx
	    	  } // bFill
	    	  d = sharpGetBuffer();
	          d += (dx/8) + (ty * LCD_PITCH);
	          ucMask = 0x80>>(dx & 7); // destination bit number for this starting point
	          for (tx=dx; tx<(dx+pGlyph->width); tx++) {
	            if (bits == 0) { // need to read more font data
	               uc = s[iBitOff>>3]; // get more font bitmap data
	               bits = 8 - (iBitOff & 7); // we might not be on a byte boundary
	               iBitOff += bits; // because of a clipped line
	               uc <<= (8-bits);
	            } // if we ran out of bits
	            if (tx < LCD_WIDTH) { // foreground pixel
	                if (uc & 0x80) {
	                   if (u8Color == 1)
	                      d[0] |= ucMask;
	                   else
	                      d[0] &= ~ucMask;
	                } else {
	                    if (u8Color == 1)
	                       d[0] &= ~ucMask;
	                    else
	                       d[0] |= ucMask;
	                }
	            }
	            bits--; // next bit
	            uc <<= 1;
		    ucMask >>= 1;
		    if (ucMask == 0) { // next destination byte
                       ucMask = 0x80;
		       d++;
		    }
	         } // for tx
	    	  if (bFill) {
				  // clear the empty part of the character rectangle (right)
	    		  d = sharpGetBuffer();
				  d += ((dx+pGlyph->width)/8) + (ty * LCD_PITCH);
				  ucMask = 0x80>>((dx+pGlyph->width) & 7);
				  for (tx=dx+pGlyph->width; tx<x+pGlyph->xAdvance; tx++) {
					  if (u8Color == 1)
						 d[0] &= ~ucMask;
					  else
						 d[0] |= ucMask;
					  ucMask >>= 1;
					  if (ucMask == 0) {
						  d++;
						  ucMask = 0x80;
					  }
				  } // for tx
	    	  } // bFill
	      } // for ty
	      x += pGlyph->xAdvance; // width of this character
	   } // while drawing characters
	   cursor_x = x;
	   cursor_y = y;
} /* sharpWriteStringCustom() */

//
// Draw a string of normal (8x8), small (6x8) or large (12x16) characters
// At the given col+row
//
int sharpWriteString(int x, int y, char *szMsg, int iSize, int bInvert)
{
int i, tx, ty, iFontOff, iLen;
unsigned char c, ucDstMask, ucSrcMask, *d, *s, uc, ucInvert;

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
       return -1; // can't draw off the display
    ucInvert = (bInvert) ? 0xff : 0x00;
    if (x == -1)
    	x = cursor_x;
    if (y == -1)
    	y = cursor_y;
    if (iSize == FONT_8x8 || iSize == FONT_6x8) // 8x8 and 6x8 font
    {
       i = 0;
       while (x < LCD_WIDTH && szMsg[i] != 0 && y < LCD_HEIGHT)
       {
             c = (unsigned char)szMsg[i];
             if (iSize == FONT_8x8) {
            	 iFontOff = (int)(c-32) * 7;
             	 s = (uint8_t *)&ucFont[iFontOff];
             	 iLen = 7;
             } else {
            	 iFontOff = (int)(c-32) * 5;
             	 s = (uint8_t *)&ucSmallFont[iFontOff];
             	 iLen = 5;
             }
             if (x + iLen > LCD_WIDTH) // clip right edge
                 iLen = LCD_WIDTH - x;
             for (tx=0; tx<iLen; tx++, x++) {
            	 uc = *s++;
            	 uc ^= ucInvert;
            	 ucSrcMask = 1;
            	 d = sharpGetBuffer();
                 d += (y * LCD_PITCH) + (x >> 3);
                 ucDstMask = 0x80 >> (x & 7);
            	 for (ty=0; ty<8; ty++) {
            		 if (uc & ucSrcMask)
            			 d[0] |= ucDstMask;
            		 else
                		 d[0] &= ~ucDstMask;
            		 d += LCD_PITCH; // next line
            		 ucSrcMask <<= 1;
            	 } // for ty
             } // for tx
             x++; // 1 pixel gap
             if (x >= LCD_WIDTH-7) // word wrap enabled?
             {
               x = 0; // start at the beginning of the next line
               y += 8;
             }
         i++;
       } // while
       cursor_x = x;
       cursor_y = y;
       return 0;
    } // 8x8
    else if (iSize == FONT_12x16) // 6x8 stretched to 12x16
    {
      i = 0;
      while (x < LCD_WIDTH && y < LCD_HEIGHT && szMsg[i] != 0)
      {
// stretch the 'normal' font instead of using the big font
              int tx, ty;
              uint8_t ucTemp[32];

              c = szMsg[i] - 32;
              unsigned char uc1, uc2, ucMask, *pDest;
              s = (unsigned char *)&ucSmallFont[(int)c*5];
              ucTemp[0] = 0; // first column is blank
              memcpy(&ucTemp[1], s, 5);
              // Stretch the font to double width + double height
              memset(&ucTemp[6], 0, 24); // write 24 new bytes
              for (tx=0; tx<6; tx++)
              {
                  ucMask = 3;
                  pDest = &ucTemp[6+tx*2];
                  uc1 = uc2 = 0;
                  c = ucTemp[tx];
                  for (ty=0; ty<4; ty++)
                  {
                      if (c & (1 << ty)) // a bit is set
                          uc1 |= ucMask;
                      if (c & (1 << (ty + 4)))
                          uc2 |= ucMask;
                      ucMask <<= 2;
                  }
                  pDest[0] = uc1;
                  pDest[1] = uc1; // double width
                  pDest[12] = uc2;
                  pDest[13] = uc2;
              }
              // smooth the diagonal lines
              for (tx=0; tx<5; tx++)
              {
                  uint8_t c0, c1, ucMask2;
                  c0 = ucTemp[tx];
                  c1 = ucTemp[tx+1];
                  pDest = &ucTemp[6+tx*2];
                  ucMask = 1;
                  ucMask2 = 2;
                  for (ty=0; ty<7; ty++)
                  {
                      if (((c0 & ucMask) && !(c1 & ucMask) && !(c0 & ucMask2) && (c1 & ucMask2)) || (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2)))
                      {
                          if (ty < 3) // top half
                          {
                              pDest[1] |= (1 << ((ty * 2)+1));
                              pDest[2] |= (1 << ((ty * 2)+1));
                              pDest[1] |= (1 << ((ty+1) * 2));
                              pDest[2] |= (1 << ((ty+1) * 2));
                          }
                          else if (ty == 3) // on the border
                          {
                              pDest[1] |= 0x80; pDest[2] |= 0x80;
                              pDest[13] |= 1; pDest[14] |= 1;
                          }
                          else // bottom half
                          {
                              pDest[13] |= (1 << (2*(ty-4)+1));
                              pDest[14] |= (1 << (2*(ty-4)+1));
                              pDest[13] |= (1 << ((ty-3) * 2));
                              pDest[14] |= (1 << ((ty-3) * 2));
                          }
                      }
                      else if (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2))
                      {
                          if (ty < 4) // top half
                          {
                              pDest[1] |= (1 << ((ty * 2)+1));
                              pDest[2] |= (1 << ((ty+1) * 2));
                          }
                          else
                          {
                              pDest[13] |= (1 << (2*(ty-4)+1));
                              pDest[14] |= (1 << ((ty-3) * 2));
                          }
                      }
                      ucMask <<= 1; ucMask2 <<= 1;
                  }
              }
              iLen = 12;
              if (x + iLen > LCD_WIDTH) // clip right edge
                  iLen = LCD_WIDTH - x;
              if (bInvert) {
                  for (tx=0; tx<iLen; tx++) {
                	  ucTemp[tx+6] = ~ucTemp[tx+6];
                	  ucTemp[tx+18] = ~ucTemp[tx+18];
                  }
              }
              for (tx=0; tx<iLen; tx++, x++) {
                  ucDstMask = 0x80 >> (x & 7);
                  ucSrcMask = 1;
                  d = sharpGetBuffer();
                  d += (y * LCD_PITCH) + (x>>3);
                  s = &ucTemp[6+tx];
                  for (ty=0; ty<8; ty++) {
                	  if (s[0] & ucSrcMask)
                		  d[0] |= ucDstMask;
                	  else
                		  d[0] &= ~ucDstMask;
               		  if (s[12] & ucSrcMask)
               			  d[LCD_PITCH*8] |= ucDstMask;
               		  else
               			  d[LCD_PITCH*8] &= ~ucDstMask;
               		  ucSrcMask <<= 1;
                	  d += LCD_PITCH;
                  } // for ty
              } // for tx
              if (x >= LCD_WIDTH-11) // word wrap enabled?
              {
                  x = 0; // start at the beginning of the next line
                  y += 16;
              }
          i++;
      } // while
      cursor_x = x;
      cursor_y = y;
      return 0;
    } // 12x16
  return -1; // invalid size
} /* sharpWriteString() */

int sharpGetCursorX(void)
{
	return cursor_x;
}
int sharpGetCursorY(void)
{
	return cursor_y;
}
