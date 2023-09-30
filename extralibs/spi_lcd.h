//
// spi_lcd.h
// a Sitronix LCD display library
// Created on: Sep 11, 2023
// written by Larry Bank (bitbank@pobox.com)
//
//
// Copyright 2023 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//

#ifndef USER_SPI_LCD_H_
#define USER_SPI_LCD_H_

#include "funconfig.h"
#include <stdint.h>
#include "ch32v_hal.h"
#include <string.h>

enum {
	LCD_ST7735_80x160 = 0,
	LCD_ST7735_80x160_B,
	LCD_ST7735_128x128,
	LCD_ST7735_128x160,
	LCD_ST7789_135x240,
	LCD_ST7789_172x320,
	LCD_ST7789_240x240,
	LCD_ST7789_240x280,
	LCD_ST7789_240x320,
	LCD_GC9107_128x128,
	LCD_ILI9341_240x320,
	LCD_COUNT
};

enum {
	ORIENTATION_0 = 0,
	ORIENTATION_90,
	ORIENTATION_180,
	ORIENTATION_270
};

#ifdef CH32V003
// 2 complete lines across a 320 pixel color LCD
#define CACHE_SIZE (320*2)
#else // larger RAM systems
#define CACHED_LINES 16
// enough memory to hold 16 lines of the display for fast character drawing
#define CACHE_SIZE (320*CACHED_LINES)
#endif
// memory offset of visible area (80x160 out of 240x320)

// Proportional font data taken from Adafruit_GFX library
/// Font data stored PER GLYPH
typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint8_t first;    ///< ASCII extents (first char)
  uint8_t last;     ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

void lcdFill(uint16_t u16Color);
void lcdInit(int iLCDType, uint32_t u32Speed, uint8_t u8CSPin, uint8_t u8DCPin, uint8_t u8RSTPin, uint8_t u8BLPin);
void lcdWriteCMD(uint8_t ucCMD);
void lcdWriteDATA(uint8_t *pData, int iLen);
int lcdWriteString(int x, int y, char *szMsg, uint16_t usFGColor, uint16_t usBGColor, int iFontSize);
int lcdDrawTile(int x, int y, int iTileWidth, int iTileHeight, unsigned char *pTile, int iPitch);
int lcdWriteStringCustom(GFXfont *pFont, int x, int y, char *szMsg, uint16_t usFGColor, uint16_t usBGColor, int bBlank);
void lcdOrientation(int iOrientation);
void lcdRectangle(int x, int y, int cx, int cy, uint16_t usColor, int bFill);
void lcdEllipse(int centerX, int centerY, int radiusX, int radiusY, uint16_t color, int bFilled);

void memset16(uint16_t *u16Dest, uint16_t u16Pattern, int iLen);
#define COLOR_BLACK 0
#define COLOR_WHITE 0xffff
#define COLOR_RED 0xf800
#define COLOR_GREEN 0x7e0
#define COLOR_BLUE 0x1f
#define COLOR_MAGENTA 0xf81f
#define COLOR_CYAN 0x7ff
#define COLOR_YELLOW 0xffe0

// MADCTL flip bits
#define MADCTL_YFLIP 0x80
#define MADCTL_XFLIP 0x40
#define MADCTL_VFLIP 0x20

enum {
	FONT_6x8 = 0,
	FONT_8x8,
	FONT_12x16,
	FONT_COUNT
};

#endif /* USER_SPI_LCD_H_ */
