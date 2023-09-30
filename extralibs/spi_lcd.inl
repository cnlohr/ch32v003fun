//
// spi_lcd.c
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

#include "spi_lcd.h"

static uint8_t u8CS, u8DC, u8BL;
static uint8_t u8MADCTL; // original value
static int iCursorX, iCursorY;
static int iNativeWidth, iNativeHeight, iNativeXOff, iNativeYOff, iLCDWidth, iLCDHeight, iLCDPitch, iLCDXOff, iLCDYOff;
static uint8_t u8Cache0[CACHE_SIZE]; // ping-pong data buffers
static uint8_t u8Cache1[CACHE_SIZE];
static uint8_t *pCache0 = u8Cache0, *pCache1 = u8Cache1;
volatile int bDMA = 0;

const uint8_t ucILI9341InitList[] = {
        4, 0xEF, 0x03, 0x80, 0x02,
        4, 0xCF, 0x00, 0XC1, 0X30,
        5, 0xED, 0x64, 0x03, 0X12, 0X81,
        4, 0xE8, 0x85, 0x00, 0x78,
        6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02,
        2, 0xF7, 0x20,
        3, 0xEA, 0x00, 0x00,
        2, 0xc0, 0x23, // Power control
        2, 0xc1, 0x10, // Power control
        3, 0xc5, 0x3e, 0x28, // VCM control
        2, 0xc7, 0x86, // VCM control2
        2, 0x36, 0x48, // Memory Access Control
        1, 0x20,        // non inverted
        2, 0x3a, 0x55,
        3, 0xb1, 0x00, 0x18,
        4, 0xb6, 0x08, 0x82, 0x27, // Display Function Control
        2, 0xF2, 0x00, // Gamma Function Disable
        2, 0x26, 0x01, // Gamma curve selected
        16, 0xe0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
            0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00, // Set Gamma
	    16, 0xe1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
            0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F, // Set Gamma
        3, 0xb1, 0x00, 0x10, // FrameRate Control 119Hz
        0
};

const uint8_t uc240x240InitList[] = {
    1, 0x13, // partial mode off
    1, 0x21, // display inversion off
    2, 0x36,0x60,    // memory access 0xc0 for 180 degree flipped
    2, 0x3a,0x55,    // pixel format; 5=RGB565
    3, 0x37,0x00,0x00, //
    6, 0xb2,0x0c,0x0c,0x00,0x33,0x33, // Porch control
    2, 0xb7,0x35,    // gate control
    2, 0xbb,0x1a,    // VCOM
    2, 0xc0,0x2c,    // LCM
    2, 0xc2,0x01,    // VDV & VRH command enable
    2, 0xc3,0x0b,    // VRH set
    2, 0xc4,0x20,    // VDV set
    2, 0xc6,0x0f,    // FR control 2
    3, 0xd0, 0xa4, 0xa1,     // Power control 1
    15, 0xe0, 0x00,0x19,0x1e,0x0a,0x09,0x15,0x3d,0x44,0x51,0x12,0x03,
        0x00,0x3f,0x3f,     // gamma 1
    15, 0xe1, 0x00,0x18,0x1e,0x0a,0x09,0x25,0x3f,0x43,0x52,0x33,0x03,
        0x00,0x3f,0x3f,        // gamma 2
    1, 0x29,    // display on
    0
};

const uint8_t ucGC9107InitList[] = {
    1, 0x13, // partial mode off
    1, 0x21, // display inversion off
    2, 0x36,0x68,    // memory access 0xc0 for 180 degree flipped
    2, 0x3a,0x55,    // pixel format; 5=RGB565
    3, 0x37,0x00,0x00, //
    6, 0xb2,0x0c,0x0c,0x00,0x33,0x33, // Porch control
    2, 0xb7,0x35,    // gate control
    2, 0xbb,0x1a,    // VCOM
    2, 0xc0,0x2c,    // LCM
    2, 0xc2,0x01,    // VDV & VRH command enable
    2, 0xc3,0x0b,    // VRH set
    2, 0xc4,0x20,    // VDV set
    2, 0xc6,0x0f,    // FR control 2
    3, 0xd0, 0xa4, 0xa1,     // Power control 1
    15, 0xe0, 0x00,0x19,0x1e,0x0a,0x09,0x15,0x3d,0x44,0x51,0x12,0x03,
        0x00,0x3f,0x3f,     // gamma 1
    15, 0xe1, 0x00,0x18,0x1e,0x0a,0x09,0x25,0x3f,0x43,0x52,0x33,0x03,
        0x00,0x3f,0x3f,        // gamma 2
    1, 0x29,    // display on
    0
};

const uint8_t uc80InitList[] = {
    2, 0x3a, 0x05,    // pixel format RGB565
    2, 0x36, 0x68, // MADCTL (0/90/180/270 and color/inversion)
    17, 0xe0, 0x09, 0x16, 0x09,0x20,
    0x21,0x1b,0x13,0x19,
    0x17,0x15,0x1e,0x2b,
    0x04,0x05,0x02,0x0e, // gamma sequence
    17, 0xe1, 0x0b,0x14,0x08,0x1e,
    0x22,0x1d,0x18,0x1e,
    0x1b,0x1a,0x24,0x2b,
    0x06,0x06,0x02,0x0f,
    1, 0x20,    // display inversion off
	1, 0x29, // display on
    0
};

const uint8_t uc160InitList[] = {
        2, 0x3a, 0x05,  // pixel format RGB565
        2, 0x36, 0x60, // MADCTL
        17, 0xe0, 0x09, 0x16, 0x09,0x20,
                0x21,0x1b,0x13,0x19,
                0x17,0x15,0x1e,0x2b,
                0x04,0x05,0x02,0x0e, // gamma sequence
        17, 0xe1, 0x0b,0x14,0x08,0x1e,
                0x22,0x1d,0x18,0x1e,
                0x1b,0x1a,0x24,0x2b,
                0x06,0x06,0x02,0x0f,
			    1, 0x20,    // display inversion off
				1, 0x29, // display on
        0
};
const uint8_t uc128InitList[] = {
        2, 0x3a, 0x05,  // pixel format RGB565
        2, 0x36, 0x68, // MADCTL
        17, 0xe0, 0x09, 0x16, 0x09,0x20,
                0x21,0x1b,0x13,0x19,
                0x17,0x15,0x1e,0x2b,
                0x04,0x05,0x02,0x0e, // gamma sequence
        17, 0xe1, 0x0b,0x14,0x08,0x1e,
                0x22,0x1d,0x18,0x1e,
                0x1b,0x1a,0x24,0x2b,
                0x06,0x06,0x02,0x0f,
			    1, 0x20,    // display inversion off
				1, 0x29, // display on
        0
};
const uint8_t ucFont[]PROGMEM = {
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
const uint8_t ucSmallFont[] PROGMEM = {
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

void DMA1_Channel3_IRQHandler(void) __attribute__((interrupt));

//
// This function gets called when the current DMA transaction completes
// We use this to disable the chip select (CS) signal on the LCD
// and clear our volatile flag (DMA busy)
//
void DMA1_Channel3_IRQHandler(void)
{
	        // why is this needed? Can't just direct compare the reg in tests below
        volatile uint16_t intfr = DMA1->INTFR;

	if (intfr & DMA1_IT_TC3) {
		DMA1->INTFCR = DMA1_IT_TC3;
		DMA1_Channel3->CFGR &= ~DMA_CFGR1_EN;
		if (u8CS != 0xff)
			digitalWrite(u8CS, 1); // de-activate CS
		bDMA = 0; // no longer active DMA transaction
	}
} /* DMA1_Channel3_IRQHandler() */

//
// Initialize the DMA channel for SPI transmit (3)
//
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1; // Enable DMA peripheral
        //DMA1_Channel3 is for SPI 
        DMA1_Channel3->PADDR = ppadr;
        DMA1_Channel3->MADDR = memadr;
        DMA1_Channel3->CNTR  = bufsize;
        DMA1_Channel3->CFGR  =
                DMA_M2M_Disable |
                DMA_Priority_VeryHigh |
                DMA_MemoryDataSize_Byte |
                DMA_PeripheralDataSize_Byte |
                DMA_MemoryInc_Enable |
                DMA_Mode_Normal |
                DMA_DIR_PeripheralDST |
                DMA_IT_TC;

        NVIC_EnableIRQ( DMA1_Channel3_IRQn );
        DMA1_Channel3->CFGR |= DMA_CFGR1_EN;
} /* DMA_Tx_Init() */
//
// Faster way to write a 16-bit pattern into memory
//
void memset16(uint16_t *u16Dest, uint16_t u16Pattern, int iLen)
{
	uint32_t *u32D, u32;
	u32 = u16Pattern | (u16Pattern << 16);
	if (((uint32_t)u16Dest & 3) == 0) {
		u32D = (uint32_t *)u16Dest;
		while (iLen >= 2) {
			*u32D++ = u32;
			iLen -= 2;
		}
		u16Dest = (uint16_t *)u32D;
	}
	while (iLen) {
		*u16Dest += u16Pattern;
		iLen--;
	}
} /* memset16() */

//
// Write a command byte to the LCD (D/C = LOW)
//
void lcdWriteCMD(uint8_t ucCMD)
{
	while (bDMA) {}; // wait for old transaction to complete
	digitalWrite(u8DC, 0);
	if (u8CS != 0xff)
		digitalWrite(u8CS, 0);
	SPI_write(&ucCMD, 1);
	if (u8CS != 0xff)
		digitalWrite(u8CS, 1);
	digitalWrite(u8DC, 1);

} /* lcdWriteCMD() */
//
// Write a block of data to the LCD (D/C = high)
// Use DMA if the block is longer than a certain threshold
//
void lcdWriteDATA(uint8_t *pData, int iLen)
{
	uint8_t *p;
	while (bDMA) {}; // wait for old transaction to complete
	if (iLen >= 32) { // arbitrary cutoff point
		if (u8CS != 0xff)
			digitalWrite(u8CS, 0); // activate CS
		DMA1_Channel3->CNTR = iLen;
		DMA1_Channel3->MADDR = (uint32_t)pCache0;

		DMA1_Channel3->CFGR |= DMA_CFGR1_EN; //		DMA_Cmd(DMA1_Channel3, ENABLE); // have DMA send the data
		bDMA = 1; // tell our code that DMA is currently active for next time
		// swap buffers
		p = pCache0;
		pCache0 = pCache1;
		pCache1 = p;
	} else {
		if (u8CS != 0xff)
			digitalWrite(u8CS, 0);
		SPI_write(pData, iLen);
		if (u8CS != 0xff)
			digitalWrite(u8CS, 1);
	}
} /* lcdWriteDATA() */

//
// Initialize the GPIOs needed to control the LCD
// and send the command sequence to configure the display
//
void lcdInit(int iLCDType, uint32_t u32Speed, uint8_t u8CSPin, uint8_t u8DCPin, uint8_t u8RSTPin, uint8_t u8BLPin)
{
//    uint8_t iBGR = 0;
	uint8_t *s = NULL;
	int iCount;

	if (iLCDType < 0 || iLCDType >= LCD_COUNT) return;
	switch (iLCDType) {
	case LCD_ST7789_135x240:
		iNativeWidth = iLCDWidth = 240; // initialize in landscape mode
		iNativeHeight = iLCDHeight = 135;
		iNativeXOff = iLCDXOff = 40;
		iNativeYOff = iLCDYOff = 53;
		u8MADCTL = 0x60;
	    s = (unsigned char *)uc240x240InitList;
		break;
	case LCD_ST7789_172x320:
		iNativeWidth = iLCDWidth = 320; // initialize in landscape mode
		iNativeHeight = iLCDHeight = 172;
		iNativeXOff = iLCDXOff = 0;
		iNativeYOff = iLCDYOff = 34;
		u8MADCTL = 0x60;
	    s = (unsigned char *)uc240x240InitList;
		break;
	case LCD_ST7789_240x280:
		iNativeWidth = iLCDWidth = 280; // initialize in landscape mode
		iNativeHeight = iLCDHeight = 240;
		iNativeXOff = iLCDXOff = 20;
		iNativeYOff = iLCDYOff = 0;
		u8MADCTL = 0x60;
	    s = (unsigned char *)uc240x240InitList;
		break;
	case LCD_ST7735_80x160:
		iNativeWidth = iLCDWidth = 160; // initialize in landscape mode
		iNativeHeight = iLCDHeight = 80;
		iNativeXOff = iLCDXOff = 0;
		iNativeYOff = iLCDYOff = 24;
		u8MADCTL = 0x68;
	    s = (unsigned char *)uc80InitList;
		break;
	case LCD_ILI9341_240x320:
		iNativeWidth = iLCDWidth = 240;
		iNativeHeight = iLCDHeight = 320;
		iNativeXOff = iLCDXOff = 0;
		iNativeYOff = iLCDYOff = 0;
		u8MADCTL = 0x60;
	    s = (unsigned char *)ucILI9341InitList;
		break;
	case LCD_GC9107_128x128:
		iNativeWidth = iLCDWidth = 128;
		iNativeHeight = iLCDHeight = 128;
		iNativeXOff = iLCDXOff = 1;
		iNativeYOff = iLCDYOff = 2;
		u8MADCTL = 0x68;
	    s = (unsigned char *)ucGC9107InitList;
		break;
	case LCD_ST7735_128x128:
		iNativeWidth = iLCDWidth = 128;
		iNativeHeight = iLCDHeight = 128;
		iNativeXOff = iLCDXOff = 1;
		iNativeYOff = iLCDYOff = 0;
		u8MADCTL = 0x68;
	    s = (unsigned char *)uc128InitList;
		break;
	case LCD_ST7735_128x160:
		iNativeWidth = iLCDWidth = 160;
		iNativeHeight = iLCDHeight = 128;
		iNativeXOff = iLCDXOff = 0;
		iNativeYOff = iLCDYOff = 0;
		u8MADCTL = 0x60;
	    s = (unsigned char *)uc160InitList;
		break;
	} // switch on LCD type
	iLCDPitch = iLCDWidth*2;
	u8CS = u8CSPin;
	if (u8CSPin != 0xff) {
		pinMode(u8CSPin, OUTPUT);
		digitalWrite(u8CSPin, 1);
	}
	pinMode(u8RSTPin, OUTPUT);
	digitalWrite(u8RSTPin, 0); // reset the display controller
	Delay_Ms(100);
	digitalWrite(u8RSTPin, 1);
	Delay_Ms(200);

	SPI_begin(u32Speed, 0);
	u8DC = u8DCPin;
	pinMode(u8DCPin, OUTPUT);
	u8BL = u8BLPin;
	pinMode(u8BL, OUTPUT);
	digitalWrite(u8BL, 1); // turn on backlight
//    if (pLCD->iLCDFlags & FLAGS_SWAP_RB)
//        iBGR = 8;
	lcdWriteCMD(0x01); // SW reset
	Delay_Ms(200);
	lcdWriteCMD(0x11); // sleep out
	Delay_Ms(100);
    iCount = 1;
     while (s && iCount)
     {
		 iCount = *s++;
		 if (iCount != 0)
		 {
			 lcdWriteCMD(s[0]);
			 lcdWriteDATA(&s[1], iCount-1);
			 s += iCount;
		 } // if count
     }// while
     DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DATAR, (u32)pCache0, 0);

} /* lcdInit() */

//
// Sitronix LCDs support 4 possible orientations through use of the
// X, Y and V mirror/flip bits
// V swaps the X/Y axis
//
void lcdOrientation(int iOrientation)
{
	uint8_t u8 = u8MADCTL; // original value

	switch (iOrientation) {
	case ORIENTATION_0: // use original MADCTL value
		iLCDWidth = iNativeWidth;
		iLCDHeight = iNativeHeight;
		iLCDPitch = iLCDWidth * 2;
		iLCDXOff = iNativeXOff;
		iLCDYOff = iNativeYOff;
		break;
	case ORIENTATION_90:
		u8 ^= MADCTL_XFLIP;
		u8 ^= MADCTL_VFLIP;
		iLCDWidth = iNativeHeight;
		iLCDHeight = iNativeWidth;
		iLCDPitch = iLCDWidth * 2;
		iLCDXOff = iNativeYOff;
		iLCDYOff = iNativeXOff;
		break;
	case ORIENTATION_180:
		u8 ^= MADCTL_XFLIP;
		u8 ^= MADCTL_YFLIP;
		iLCDWidth = iNativeWidth;
		iLCDHeight = iNativeHeight;
		iLCDPitch = iLCDWidth * 2;
		iLCDXOff = iNativeXOff;
		iLCDYOff = iNativeYOff;
		break;
	case ORIENTATION_270:
		u8 ^= MADCTL_YFLIP;
		u8 ^= MADCTL_VFLIP;
		iLCDWidth = iNativeHeight;
		iLCDHeight = iNativeWidth;
		iLCDPitch = iLCDWidth * 2;
		iLCDXOff = iNativeYOff;
		iLCDYOff = iNativeXOff;
		break;
	}
	 lcdWriteCMD(0x36); // MADCTL
	 lcdWriteDATA(&u8, 1);
} /* lcdOrientation() */

//
// Set the memory window (AKA write position)
//
void lcdSetPosition(int x, int y, int w, int h)
{
uint8_t ucBuf[8];

     x += iLCDXOff;
     y += iLCDYOff;
     ucBuf[0] = (unsigned char)(x >> 8);
     ucBuf[1] = (unsigned char)x;
     x = x + w - 1;
     ucBuf[2] = (unsigned char)(x >> 8);
     ucBuf[3] = (unsigned char)x;
     lcdWriteCMD(0x2a); // column address cmd
     lcdWriteDATA(ucBuf, 4);
     ucBuf[0] = (unsigned char)(y >> 8);
     ucBuf[1] = (unsigned char)y;
     y = y + h - 1;
     ucBuf[2] = (unsigned char)(y >> 8);
     ucBuf[3] = (unsigned char)y;
     lcdWriteCMD(0x2b); // row address cmd
     lcdWriteDATA(ucBuf, 4);
     lcdWriteCMD(0x2c); // RAMWR - start writing
} /* lcdSetPosition() */

//
// For drawing ellipses, a circle is drawn and the x and y pixels are scaled by a 16-bit integer fraction
// This function draws a single pixel and scales its position based on the x/y fraction of the ellipse
//
void DrawScaledPixel(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint16_t usColor)
{
    if (iXFrac != 0x10000) x = (x * iXFrac) >> 16;
    if (iYFrac != 0x10000) y = (y * iYFrac) >> 16;
    x += iCX; y += iCY;
    if (x < 0 || x >= iLCDWidth || y < 0 || y >= iLCDHeight)
        return; // off the screen
    lcdSetPosition(x, y, 1, 1);
    lcdWriteDATA((uint8_t *)&usColor, 2);
} /* DrawScaledPixel() */
//
// Draw the given x/y symmetrically across the center point
// as a continuous line
//
void DrawScaledLine(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint16_t usColor)
{
    int32_t iLen, x2;
    if (iXFrac != 0x10000) x = (x * iXFrac) >> 16;
    if (iYFrac != 0x10000) y = (y * iYFrac) >> 16;
    x2 = iCX + x;
    x = iCX - x;
    y += iCY;
    if (x < 0) x = 0;
    if (x2 >= iLCDWidth) x2 = iLCDWidth-1;
    iLen = x2 - x + 1; // new length
    lcdSetPosition(x, y, iLen, 1);
    memset16((uint16_t *)pCache0, usColor, iLen);
    lcdWriteDATA(pCache0, iLen*2);
} /* DrawScaledLine() */
//
// Draw the 8 pixels around the Bresenham circle
// (scaled to make an ellipse)
//
void BresenhamCircle(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint16_t usColor, int bFill)
{
    if (bFill) // draw a filled ellipse
    {
        // for a filled ellipse, draw 4 lines instead of 8 pixels
        DrawScaledLine(iCX, iCY, y, x, iXFrac, iYFrac, usColor);
        DrawScaledLine(iCX, iCY, y, -x, iXFrac, iYFrac, usColor);
        DrawScaledLine(iCX, iCY, x, y, iXFrac, iYFrac, usColor);
        DrawScaledLine(iCX, iCY, x, -y, iXFrac, iYFrac, usColor);
    }
    else // draw 8 pixels around the edges
    {
        DrawScaledPixel(iCX, iCY, x, y, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, -x, y, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, x, -y, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, -x, -y, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, y, x, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, -y, x, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, y, -x, iXFrac, iYFrac, usColor);
        DrawScaledPixel(iCX, iCY, -y, -x, iXFrac, iYFrac, usColor);
    }
} /* BresenhamCircle() */
//
// Draw an ellipse (a circle if Rx==Ry)
//
void lcdEllipse(int iCenterX, int iCenterY, int iRadiusX, int iRadiusY, uint16_t usColor, int bFill)
{
    int32_t iRadius, iXFrac, iYFrac;
    int32_t iDelta, x, y;

    if (iRadiusX > iRadiusY) // use X as the primary radius
    {
        iRadius = iRadiusX;
        iXFrac = 65536;
        iYFrac = (iRadiusY * 65536) / iRadiusX;
    }
    else
    {
        iRadius = iRadiusY;
        iXFrac = (iRadiusX * 65536) / iRadiusY;
        iYFrac = 65536;
    }
    usColor = __builtin_bswap16(usColor); // swap byte order
    iDelta = 3 - (2 * iRadius);
    x = 0; y = iRadius;
    while (x < y)
    {
        BresenhamCircle(iCenterX, iCenterY, x, y, iXFrac, iYFrac, usColor, bFill);
        x++;
        if (iDelta < 0)
        {
            iDelta += (4*x) + 6;
        }
        else
        {
            iDelta += 4 * (x-y) + 10;
            y--;
        }
    }
} /* lcdEllipse() */
//
// Draw an outline or filled rectangle
// requesting a rectangle which draws past an edge will result it in being clipped to the edge
//
void lcdRectangle(int x, int y, int cx, int cy, uint16_t usColor, int bFill)
{
	int ty;
	if (x >= iLCDWidth || y >= iLCDHeight) return; // not visible
	if (x < 0) {
		cx += x;
		x = 0;
	} else if (x + cx > iLCDWidth) cx = iLCDWidth - cx;
	if (y < 0) {
		cy += y;
		y = 0;
	} else if (y + cy > iLCDHeight) cy = iLCDHeight - cy;
	usColor = __builtin_bswap16(usColor);
	if (bFill) { // draw a filled rectangle
		lcdSetPosition(x, y, cx, cy);
		for (ty = 0; ty < cy; ty++) {
			if (ty < 2) memset16((uint16_t *)pCache0, usColor, cx); // need to do it 2 times for buffers
			lcdWriteDATA(pCache0, cx*2);
		} // for ty
	} else { // outline rectangle
		// each side is drawn as a straight line
		lcdSetPosition(x, y, cx, 1); // top line
		memset16((uint16_t *)pCache0, usColor, cx);
		lcdWriteDATA(pCache0, cx*2);
		lcdSetPosition(x, y+cy-1, cx, 1); // bottom line
		memset16((uint16_t *)pCache0, usColor, cx);
		lcdWriteDATA(pCache0, cx*2);
		lcdSetPosition(x, y, 1, cy); // left line
		memset16((uint16_t *)pCache0, usColor, cy);
		lcdWriteDATA(pCache0, cy*2);
		lcdSetPosition(x+cx-1, y, 1, cy); // right line
		memset16((uint16_t *)pCache0, usColor, cy);
		lcdWriteDATA(pCache0, cy*2);
	}
} /* lcdRectangle() */
//
// Draw a NxN RGB565 tile
// This reverses the pixel byte order and sets a memory "window"
// of pixels so that the write can occur in one shot
//
int lcdDrawTile(int x, int y, int iTileWidth, int iTileHeight, unsigned char *pTile, int iPitch)
{
    int i, j;
    uint16_t *s16, *d16;

    if (iTileWidth*iTileHeight*2 > CACHE_SIZE) {
        return -1; // tile must fit in SPI cache
    }
    // First convert to big-endian order
    d16 = (uint16_t *)pCache0;
    for (j=0; j<iTileHeight; j++)
    {
        s16 = (uint16_t*)&pTile[j*iPitch];
        for (i=0; i<iTileWidth; i++)
        {
            *d16++ = __builtin_bswap16(*s16++);
        } // for i;
    } // for j
    lcdSetPosition(x, y, iTileWidth, iTileHeight);
    lcdWriteDATA(pCache0, iTileWidth*iTileHeight*2);
    return 0;
} /* lcdDrawTile() */
//
// Fill the display with a solid color
//
void lcdFill(uint16_t usData)
{
	int cx, cy;
	uint16_t *d;

    usData = (usData >> 8) | (usData << 8); // swap hi/lo byte for LCD
    lcdSetPosition(0,0, iLCDWidth, iLCDHeight);
    // fit within our temp buffer
    for (cy = 0; cy < iLCDHeight; cy++) {
    	if (cy < 2) { // both buffers need a copy
    		d = (uint16_t *)pCache0; // pointer swapped after each write
    		for (cx = 0; cx < iLCDWidth; cx++) {
    			d[cx] = usData;
    		}
    	}
        lcdWriteDATA(pCache0, iLCDWidth*2); // fill with data words
   } // for y

} /* lcdFill() */

//
// Draw a 1-bpp pattern with the given color and translucency
// 1 bits are drawn as color, 0 are transparent
// The translucency value can range from 1 (barely visible) to 32 (fully opaque)
// If there is a backbuffer, the bitmap is draw only into memory
// If there is no backbuffer, the bitmap is drawn on the screen with a black background color
//
void spilcdDrawPattern(uint8_t *pPattern, int iSrcPitch, int iDestX, int iDestY, int iCX, int iCY, uint16_t usColor)
{
    int x, y;
    uint8_t *s, uc, ucMask;
    uint16_t *d, u16Clr;

     if (iDestX+iCX > iLCDWidth) // trim to fit on display
         iCX = (iLCDWidth - iDestX);
     if (iDestY+iCY > iLCDHeight)
         iCY = (iLCDHeight - iDestY);
     if (pPattern == NULL || iDestX < 0 || iDestY < 0 || iCX <=0 || iCY <= 0)
         return;
       u16Clr = (usColor >> 8) | (usColor << 8); // swap low/high bytes
       lcdSetPosition(iDestX, iDestY, iCX, iCY);
       for (y=0; y<iCY; y++)
       {
         s = &pPattern[y * iSrcPitch];
         ucMask = uc = 0;
         d = (uint16_t *)pCache0;
         for (x=0; x<iCX; x++)
         {
             ucMask >>= 1;
             if (ucMask == 0)
             {
                 ucMask = 0x80;
                 uc = *s++;
             }
             if (uc & ucMask) // active pixel
                *d++ = u16Clr;
             else
                *d++ = 0;
         } // for x
         lcdWriteDATA(pCache0, iCX*2);
       } // for y
} /* spilcdDrawPattern() */

//
// Draw a string of text with the built-in fonts
//
#if (CACHE_SIZE < 4096)
// This slower version uses one DMA transaction per character
int lcdWriteString(int x, int y, char *szMsg, uint16_t usFGColor, uint16_t usBGColor, int iFontSize)
{
int i, j, k, iLen;
unsigned char *s;
unsigned short usFG = (usFGColor >> 8) | (usFGColor << 8);
unsigned short usBG = (usBGColor >> 8) | (usBGColor << 8);

    if (x == -1)
        x = iCursorX;
    if (y == -1)
        y = iCursorY;
    if (x < 0 || y < 0) return -1;
	iLen = strlen(szMsg);
    if (iFontSize == FONT_8x8 || iFontSize == FONT_6x8) // draw the 6x8 or 8x8 font
	{
		uint16_t *usD;
        int cx;
        uint8_t *pFont;

        cx = (iFontSize == FONT_8x8) ? 8:6;
        pFont = (iFontSize == FONT_8x8) ? (uint8_t *)ucFont : (uint8_t *)ucSmallFont;
		if ((cx*iLen) + x > iLCDWidth) iLen = (iLCDWidth - x)/cx; // can't display it all
		if (iLen < 0)return -1;

		for (i=0; i<iLen; i++)
		{
			s = &pFont[((unsigned char)szMsg[i]-32) * (cx-1)];
			usD = (uint16_t *)pCache0;
            lcdSetPosition(x+(i*cx), y, cx, 8);
            uint8_t ucMask = 1;
            for (k=0; k<8; k++) // for each scanline
            {
				for (j=0; j<cx-1; j++)
				{
					if (s[j] & ucMask)
						*usD++ = usFG;
					else
						*usD++ = usBG;
				} // for j
				*usD++ = usBG; // last column is blank
                ucMask <<= 1;
            } // for k
            // write the data in one shot
            lcdWriteDATA(pCache0, cx*2*8);
		} // for each character
        x += (i*cx);
    } // 6x8 and 8x8
    if (iFontSize == FONT_12x16) // 6x8 stretched to 12x16 (with smoothing)
    {
        uint16_t *usD;

        if ((12*iLen) + x > iLCDWidth) iLen = (iLCDWidth - x)/12; // can't display it all
        if (iLen < 0) return -1;

        for (i=0; i<iLen; i++)
        {
            s = (uint8_t *)&ucSmallFont[((unsigned char)szMsg[i]-32) * 5];
            usD = (uint16_t *)pCache0;;
            lcdSetPosition(x+(i*12), y, 12, 16);
            uint8_t ucMask = 1;
            for (k=0; k<12*16; k++)
               usD[k] = usBG;
            for (k=0; k<8; k++) // for each scanline
            {
				uint8_t c0, c1;
				for (j=0; j<5; j++)
				{
					c0 = s[j];
					if (c0 & ucMask)
					   usD[0] = usD[1] = usD[12] = usD[13] = usFG;
					// test for smoothing diagonals
					if (k < 7 && j < 5) {
					   uint8_t ucMask2 = ucMask << 1;
					   c1 = s[j+1];
					   if ((c0 & ucMask) && (~c1 & ucMask) && (~c0 & ucMask2) && (c1 & ucMask2)) // first diagonal condition
						   usD[14] = usD[25] = usFG;
					   else if ((~c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && (~c1 & ucMask2))
						   usD[13] = usD[26] = usFG;
					} // if not on last row and last col
					usD+=2;
				} // for j
				usD[0] = usD[1] = usD[12] = usD[13] = usBG; // last column is blank
				usD += 2;
                usD += 12; // skip the extra line
                ucMask <<= 1;
            } // for k
        // write the data in one shot
        lcdWriteDATA(pCache0, 12*16*2);
        } // for each character
        x += i*12;
    } // FONT_12x16
    iCursorX = x;
    iCursorY = y;
	return 0;
} /* lcdWriteString() */
#else
// This faster version uses one DMA transaction per line of text
int lcdWriteString(int x, int y, char *szMsg, uint16_t usFGColor, uint16_t usBGColor, int iFontSize)
{
int i, j, k, iLen;
int iStride;
uint8_t *s;
uint16_t usFG = (usFGColor >> 8) | ((usFGColor & -1)<< 8);
uint16_t usBG = (usBGColor >> 8) | ((usBGColor & -1)<< 8);
uint16_t *usD;
int cx;
uint8_t *pFont;

    if (iFontSize < 0 || iFontSize >= FONT_COUNT)
        return -1; // invalid size
    if (x == -1)
        x = iCursorX;
    if (y == -1)
        y = iCursorY;
    if (x < 0) return -1;
    iLen = strlen(szMsg);

    if (iFontSize == FONT_12x16) {
        if ((12*iLen) + x > iLCDWidth) iLen = (iLCDWidth - x)/12; // can't display it all
        if (iLen < 0) return -1;
        iStride = iLen*12;
        lcdSetPosition(x, y, iStride, 16);
        usD = (uint16_t *)pCache0;
        for (i=0; i<iStride*16; i++)
           usD[i] = usBG; // set to background color first
        for (k = 0; k<8; k++) { // create a pair of scanlines from each original
           uint8_t ucMask = (1 << k);
           usD = (unsigned short *)&pCache0[k*iStride*4];
           for (i=0; i<iLen; i++)
           {
               uint8_t c0, c1;
               s = (uint8_t *)&ucSmallFont[((unsigned char)szMsg[i]-32) * 5];
               for (j=1; j<6; j++)
               {
                   uint8_t ucMask1 = ucMask << 1;
                   uint8_t ucMask2 = ucMask >> 1;
                   c0 = s[j-1];
                   if (c0 & ucMask)
                      usD[0] = usD[1] = usD[iStride] = usD[iStride+1] = usFG;
                   // test for smoothing diagonals
                   if (j < 5) {
                      c1 = s[j];
                      if ((c0 & ucMask) && (~c1 & ucMask) && (~c0 & ucMask1) && (c1 & ucMask1)) { // first diagonal condition
                          usD[iStride+2] = usFG;
                      } else if ((~c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask1) && (~c1 & ucMask1)) { // second condition
                          usD[iStride+1] = usFG;
                      }
                      if ((c0 & ucMask2) && (~c1 & ucMask2) && (~c0 & ucMask) && (c1 & ucMask)) { // repeat for previous line
                          usD[1] = usFG;
                      } else if ((~c0 & ucMask2) && (c1 & ucMask2) && (c0 & ucMask) && (~c1 & ucMask)) {
                          usD[2] = usFG;
                      }
                   }
                   usD+=2;
               } // for j
               usD += 2; // leave "6th" column blank
            } // for each character
        } // for each scanline
        lcdWriteDATA(pCache0, iStride*32);
        return 0;
    } // 12x16

    cx = (iFontSize == FONT_8x8) ? 8:6;
    pFont = (iFontSize == FONT_8x8) ? (uint8_t *)ucFont : (uint8_t *)ucSmallFont;
    if ((cx*iLen) + x > iLCDWidth) iLen = (iLCDWidth - x)/cx; // can't display it all
    iStride = iLen * cx*2;
    for (i=0; i<iLen; i++)
    {
        s = &pFont[((unsigned char)szMsg[i]-32) * (cx-1)];
        uint8_t ucMask = 1;
        for (k=0; k<8; k++) // for each scanline
        {
            usD = (unsigned short *)&pCache0[(k*iStride) + (i * cx*2)];
            for (j=0; j<cx-1; j++)
            {
                if (s[j] & ucMask)
                    *usD++ = usFG;
                else
                    *usD++ = usBG;
            } // for j
	    *usD++ = usBG; // blank column
            ucMask <<= 1;
        } // for k
    } // for i
    // write the data in one shot
    lcdSetPosition(x, y, cx*iLen, 8);
    lcdWriteDATA(pCache0, iLen*cx*16);
    iCursorX = x + (cx*iLen);
    iCursorY = y;
    return 0;
} /* lcdWriteString() */
#endif // cache size
//
// Draw a string in a proportional font you supply
//
int lcdWriteStringCustom(GFXfont *pFont, int x, int y, char *szMsg, uint16_t usFGColor, uint16_t usBGColor, int bBlank)
{
int i, /*j, iLen, */ k, dx, dy, cx, cy, c, iBitOff;
int tx, ty;
uint8_t *s, bits, uc;
GFXfont font;
GFXglyph glyph, *pGlyph;
#define TEMP_BUF_SIZE 64
#define TEMP_HIGHWATER (TEMP_BUF_SIZE-8)
uint16_t *d;

   if (pFont == NULL)
      return -1;
    if (x == -1)
        x = iCursorX;
    if (y == -1)
        y = iCursorY;
    if (x < 0)
        return -1;
   // in case of running on AVR, get copy of data from FLASH
   memcpy(&font, pFont, sizeof(font));
   pGlyph = &glyph;
   usFGColor = (usFGColor >> 8) | (usFGColor << 8); // swap h/l bytes
   usBGColor = (usBGColor >> 8) | (usBGColor << 8);

   i = 0;
   while (szMsg[i] && x < iLCDWidth)
   {
      c = szMsg[i++];
      if (c < font.first || c > font.last) // undefined character
         continue; // skip it
      c -= font.first; // first char of font defined
      memcpy_P(&glyph, &font.glyph[c], sizeof(glyph));
      // set up the destination window (rectangle) on the display
      dx = x + pGlyph->xOffset; // offset from character UL to start drawing
      dy = y + pGlyph->yOffset;
      cx = pGlyph->width;
      cy = pGlyph->height;
      iBitOff = 0; // bitmap offset (in bits)
      if (dy + cy > iLCDHeight)
         cy = iLCDHeight - dy; // clip bottom edge
      else if (dy < 0) {
         cy += dy;
         iBitOff += (pGlyph->width * (-dy));
         dy = 0;
      }
      if (dx + cx > iLCDWidth)
         cx = iLCDWidth - dx; // clip right edge
      s = font.bitmap + pGlyph->bitmapOffset; // start of bitmap data
      // Bitmap drawing loop. Image is MSB first and each pixel is packed next
      // to the next (continuing on to the next character line)
      bits = uc = 0; // bits left in this font byte

      if (bBlank) { // erase the areas around the char to not leave old bits
         int miny, maxy;
         c = '0' - font.first;
         miny = y + pGlyph->yOffset;
         c = 'y' - font.first;
         maxy = miny + pGlyph->height;
         if (maxy > iLCDHeight)
            maxy = iLCDHeight;
         cx = pGlyph->xAdvance;
         if (cx + x > iLCDWidth) {
            cx = iLCDWidth - x;
         }
         lcdSetPosition(x, miny, cx, maxy-miny);
            // blank out area above character
//            cy = font.yAdvance - pGlyph->height;
//            for (ty=miny; ty<miny+cy && ty < maxy; ty++) {
//               for (tx=0; tx<cx; tx++)
//                  u16Temp[tx] = usBGColor;
//               myspiWrite(pLCD, (uint8_t *)u16Temp, cx*sizeof(uint16_t), MODE_DATA, iFlags);
//            } // for ty
            // character area (with possible padding on L+R)
            for (ty=0; ty<pGlyph->height && ty+miny < maxy; ty++) {
               d = (uint16_t *)pCache0;
               for (tx=0; tx<pGlyph->xOffset && tx < cx; tx++) { // left padding
                  *d++ = usBGColor;
               }
            // character bitmap (center area)
               for (tx=0; tx<pGlyph->width; tx++) {
                  if (bits == 0) { // need more data
                     uc = s[iBitOff>>3];
                     bits = 8;
                     iBitOff += bits;
                  }
                  if (tx + pGlyph->xOffset < cx) {
                     *d++ = (uc & 0x80) ? usFGColor : usBGColor;
                  }
                  bits--;
                  uc <<= 1;
               } // for tx
               // right padding
               k = pGlyph->xAdvance - (int)(d - (uint16_t*)pCache0); // remaining amount
               for (tx=0; tx<k && (tx+pGlyph->xOffset+pGlyph->width) < cx; tx++)
                  *d++ = usBGColor;
               lcdWriteDATA(pCache0, cx*sizeof(uint16_t));
            } // for ty
            // padding below the current character
            ty = y + pGlyph->yOffset + pGlyph->height;
            for (; ty < maxy; ty++) {
            	d = (uint16_t *)pCache0;
               for (tx=0; tx<cx; tx++)
                  d[tx] = usBGColor;
               lcdWriteDATA(pCache0, cx*sizeof(uint16_t));
            } // for ty
      } else if (usFGColor == usBGColor) { // transparent
          int iCount; // opaque pixel count
          d = (uint16_t*)pCache0;
          for (iCount=0; iCount < cx; iCount++)
              d[iCount] = usFGColor; // set up a line of solid color
          iCount = 0; // number of sequential opaque pixels
             for (ty=0; ty<cy; ty++) {
             for (tx=0; tx<pGlyph->width; tx++) {
                if (bits == 0) { // need to read more font data
                   uc = s[iBitOff>>3]; // get more font bitmap data
                   bits = 8 - (iBitOff & 7); // we might not be on a byte boundary
                   iBitOff += bits; // because of a clipped line
                   uc <<= (8-bits);
                } // if we ran out of bits
                if (tx < cx) {
                    if (uc & 0x80) {
                        iCount++; // one more opaque pixel
                    } else { // any opaque pixels to write?
                        if (iCount) {
                            lcdSetPosition(dx+tx-iCount, dy+ty, iCount, 1);
                       d = (uint16_t *)pCache0; // point to start of output buffer
                          lcdWriteDATA(pCache0, iCount*sizeof(uint16_t));
                            iCount = 0;
                        } // if opaque pixels to write
                    } // if transparent pixel hit
                }
                bits--; // next bit
                uc <<= 1;
             } // for tx
             } // for ty
       // quicker drawing
      } else { // just draw the current character box fast
         lcdSetPosition(dx, dy, cx, cy);
            d = (uint16_t *)pCache0; // point to start of output buffer
            for (ty=0; ty<cy; ty++) {
            for (tx=0; tx<pGlyph->width; tx++) {
               if (bits == 0) { // need to read more font data
                  uc = s[iBitOff>>3]; // get more font bitmap data
                  bits = 8 - (iBitOff & 7); // we might not be on a byte boundary
                  iBitOff += bits; // because of a clipped line
                  uc <<= (8-bits);
                  k = (int)(d-(uint16_t*)pCache0); // number of words in output buffer
                  if (k >= TEMP_HIGHWATER) { // time to write it
                     lcdWriteDATA(pCache0, k*sizeof(uint16_t));
                     d = (uint16_t*)pCache0;
                  }
               } // if we ran out of bits
               if (tx < cx) {
                  *d++ = (uc & 0x80) ? usFGColor : usBGColor;
               }
               bits--; // next bit
               uc <<= 1;
            } // for tx
            } // for ty
            k = (int)(d-(uint16_t*)pCache0);
            if (k) // write any remaining data
               lcdWriteDATA(pCache0, k*sizeof(uint16_t));
      } // quicker drawing
      x += pGlyph->xAdvance; // width of this character
   } // while drawing characters
    iCursorX = x;
    iCursorY = y;
   return 0;
} /* lcdWriteStringCustom() */
// end of spi_lcd.inl
