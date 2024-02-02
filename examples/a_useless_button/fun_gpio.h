/*
 * Copied and Adapt from ch32v_hal.h by Larry Bank
 */


#include "ch32v003fun.h"
#include <stdio.h>
// GPIO pin states
enum {
	OUTPUT = 0,
	INPUT,
	INPUT_PULLUP,
	INPUT_PULLDOWN
};

void pinMode(uint8_t u8Pin, int iMode)
{
	GPIO_TypeDef *pGPIO;

   if (u8Pin < 0xa0 || u8Pin > 0xdf) return; // invalid pin number

	switch (u8Pin & 0xf0) {
      case 0xa0:
         RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
         pGPIO = GPIOA;
         break;
      case 0xc0:
         RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
         pGPIO = GPIOC;
         break;
      case 0xd0:
         RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
         pGPIO = GPIOD;
         break;
	}
	u8Pin &= 0xf; // isolate the pin from this port
	pGPIO->CFGLR &= ~(0xf << (4 * u8Pin)); // unset all flags

	switch (iMode) {
	case OUTPUT:
		pGPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4*u8Pin);
		break;
	case INPUT:
		pGPIO->CFGLR |= (GPIO_CNF_IN_FLOATING << (4*u8Pin));
		break;
	case INPUT_PULLUP:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4*u8Pin));
		pGPIO->BSHR = (1 << u8Pin);
		break;
	case INPUT_PULLDOWN:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4 * u8Pin));
		pGPIO->BCR = (1 << u8Pin);
		break;
    } // switch on iMode
} /* pinMode() */

void digitalWrite(uint8_t u8Pin, uint8_t u8Value)
{
	uint32_t u32Value = 1 << (u8Pin & 0xf); // turn on bit
	if (!u8Value)
		u32Value <<= 16; // turn off bit 

	switch (u8Pin & 0xf0) {
      case 0xa0: GPIOA->BSHR = u32Value; break;
      case 0xc0: GPIOC->BSHR = u32Value; break;
      case 0xd0: GPIOD->BSHR = u32Value; break;
	}
} /* digitalWrite() */

uint8_t digitalRead(uint8_t u8Pin)
{
	uint32_t u32GPIO = 1 << (u8Pin & 0xf);
	uint32_t u32Value = 0;

	switch (u8Pin & 0xf0) {
      case 0xa0: u32Value = GPIOA->INDR & u32GPIO; break;
      case 0xc0: u32Value = GPIOC->INDR & u32GPIO; break;
      case 0xd0: u32Value = GPIOD->INDR & u32GPIO; break;
	}
	return (u32Value != 0);
} /* digitalRead() */