/*
 * sharp_lcd.h
 *
 *  Created on: Jan 27, 2023
 *      Author: Larry Bank
 */

#ifndef USER_SHARP_H_
#define USER_SHARP_H_

#define LCD_WIDTH 160
#define LCD_HEIGHT 68
// add 2 bytes to the pitch for the line number and stop byte of each line
#define LCD_PITCH ((LCD_WIDTH>>3)+2)

#if !defined( _ADAFRUIT_GFX_H ) && !defined( _GFXFONT_H_ )
// 3 possible font sizes: 6x8, 8x8, 12x16 (stretched+smoothed from 6x8)
enum {
   FONT_6x8 = 0,
   FONT_8x8,
   FONT_12x16,
   FONT_COUNT
};

// Proportional font data taken from Adafruit_GFX library
/// Font data stored PER GLYPH
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

uint8_t * sharpGetBuffer(void);
void sharpUpdate(uint32_t u32Mask);
void sharpRotation(int iAngle);
void sharpInit(int iSpeed, uint8_t u8CS);
void sharpFill(uint8_t u8Pattern);
void sharpInvert(void);
void sharpDrawSprite(int x, int y, int cx, int cy, uint8_t *pData, int iPitch, int bInvert);
void sharpWriteBuffer(void);
int sharpGetCursorX(void);
int sharpGetCursorY(void);
void sharpVLine(int x, int y1, int y2, int color);
void sharpHLine(int x1, int x2, int y, int color);
int sharpWriteString(int x, int y, char *szMsg, int iSize, int bInvert);
void sharpWriteStringCustom(const GFXfont *pFont, int x, int y, char *szMsg, uint8_t ucColor, int bFill);

#endif /* USER_SHARP_H_ */
