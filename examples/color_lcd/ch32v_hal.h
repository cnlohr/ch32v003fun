//
// CH32V Hardware Abstraction layer
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
#ifndef CH32V_HAL_H_
#define CH32V_HAL_H_

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdint.h>

#define BITBANG
// GPIO pin states
enum {
	OUTPUT = 0,
	INPUT,
	INPUT_PULLUP,
	INPUT_PULLDOWN
};

#define PROGMEM
#define memcpy_P memcpy
#define pgm_read_byte(s) *(uint8_t *)s
#define pgm_read_word(s) *(uint16_t *)s

// Wrapper methods
void delay(int i);
//
// Digital pin functions use a numbering scheme to make it easier to map the
// pin number to a port name and number
// The GPIO ports A-D become the most significant nibble of the pin number
// for example, to use Port C pin 7 (C7), use the pin number 0xC7
//
void pinMode(uint8_t u8Pin, int iMode);
uint8_t digitalRead(uint8_t u8Pin);
void digitalWrite(uint8_t u8Pin, uint8_t u8Value);

// The Wire library is a C++ class; I've created a work-alike to my
// BitBang_I2C API which is a set of C functions to simplify I2C
void I2CInit(uint8_t u8SDA, uint8_t u8SCL, int iSpeed);
void I2CWrite(uint8_t u8Addr, uint8_t *pData, int iLen);
int I2CRead(uint8_t u8Addr, uint8_t *pData, int iLen);
void I2CReadRegister(uint8_t u8Addr, uint8_t u8Reg, uint8_t *pData, int iLen);
int I2CTest(uint8_t u8Addr);
void I2CSetSpeed(int iSpeed);

// SPI1 (polling mode)
void SPI_write(uint8_t *pData, int iLen);
void SPI_begin(int iSpeed, int iMode);


// Random stuff
void Standby82ms(uint8_t iTicks);
void breatheLED(uint8_t u8Pin, int iPeriod);


#endif /* CH32V_HAL_H_ */
