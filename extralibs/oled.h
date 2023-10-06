//
// SSD1306 OLED display library
// written by Larry Bank
// bitbank@pobox.com
// Copyright (c) 2023 BitBank Software, Inc.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef USER_OLED_H_
#define USER_OLED_H_

// 4 possible font sizes: 8x8, 16x32, 6x8, 16x16 (stretched from 8x8)
enum {
   FONT_6x8 = 0,
   FONT_8x8,
   FONT_12x16,
   FONT_16x16,
   FONT_16x32
};

// Debug - only support the 128x64 SSD1306 for now
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Proportional font data taken from Adafruit_GFX library
/// Font data stored PER GLYPH
#if !defined( _ADAFRUIT_GFX_H ) && !defined( _GFXFONT_H_ )
#define _GFXFONT_H_
typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint16_t first;    ///< ASCII extents (first char)
  uint16_t last;     ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;
#endif // _ADAFRUIT_GFX_H

// public methods
void oledInit(uint8_t u8Addr, int iSpeed);
void oledSetPosition(int x, int y);
void oledFill(uint8_t ucData);
void oledContrast(uint8_t cont);
void oledDrawSprite(int x, int y, int cx, int cy, uint8_t *pData, int iPitch, int bInvert);
int oledWriteString(int x, int y, const char *szMsg, int iSize, int bInvert);
void oledWriteStringCustom(const GFXfont *pFont, int x, int y, const char *szMsg, uint8_t ucColor);
void oledClearLine(int y);
int oledGetCursorX(void);
int oledGetCursorY(void);
void oledPower(int bOn);
#endif /* USER_OLED_H_ */
