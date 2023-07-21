// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 24000000

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

void RamFunction() __attribute__((naked));
void RamFunction()
{
	asm volatile("\n\
		li a0, 1 | (1<<4)\n\
		li a1, (1<<16) | (1<<(16+4))\n\
		la a2, 0x40011410\n\
1:\n\
		c.sw a0, 0(a2)\n\
		c.sw a1, 0(a2)\n\
		c.sw a0, 0(a2)\n\
		c.sw a1, 0(a2)\n\
		c.sw a0, 0(a2)\n\
		c.sw a1, 0(a2)\n\
		c.sw a0, 0(a2)\n\
		c.sw a1, 0(a2)\n\
		j 1b" );
}

uint8_t rambuffer[128];

int main()
{
	EXTEN->EXTEN_CTR = EXTEN_LDO_TRIM; // Boost LDO.
	SystemInitHSEPLL( RCC_HSEBYP );
	// When running from RAM appears to go up to about 96MHz.

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

memcpy( rambuffer, RamFunction, 128 );
	void (*fn)() = (void*) rambuffer;
	fn();
}
