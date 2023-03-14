/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;



// Tell the compiler to put this code in the .data section.  That
// will cause the startup code to copy it from flash into RAM where
// it can be easily modified at runtime.
void SRAMCode( ) __attribute__(( section(".data"))) __attribute__((noinline)) __attribute__((noreturn));
void SRAMCode( )
{
	asm volatile( 
"li a0, 0x40011410\n"
"li a1, (1 | (1<<4))\n"
"li a2, (1 | (1<<4))<<16\n"
"1: c.sw a1, 0(a0)\n"
"   c.sw a2, 0(a0)\n"
"   j 1b\n" );
}

int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	// Boost CPU supply.
	EXTEN->EXTEN_CTR = EXTEN_LDO_TRIM;

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	SRAMCode();
}

