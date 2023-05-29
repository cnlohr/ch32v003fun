// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

// You can override the interrupt vector this way:
void InterruptVector()         __attribute__((naked)) __attribute((section(".init")));
void InterruptVector()
{
	asm volatile( "\n\
	.align  2\n\
	.option   push;\n\
	.option   norvc;\n\
	j handle_reset\n\
	.option pop");
}

uint32_t count;

int main()
{
	SystemInit48HSI();

	// From here, you can do whatever you'd like!
	// This code will live up at 0x1ffff000.

	// Enable GPIOD + C
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D4 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C0 Push-Pull, 10MHz Output
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	static const uint32_t marker[] = { 0xaaaaaaaa };
	count = marker[0];

	int i;

	// Make a clear signature.
	for( i = 0; i < 10; i++ )
	{
		GPIOD->BSHR = 1 | (1<<4);                // Turn on GPIOD0 + D4
		GPIOC->BSHR = 1;                         // Turn on GPIOC0
		GPIOD->BSHR = (1<<16) | (1<<(16+4));     // Turn off GPIOD0 + D4
		GPIOC->BSHR = 1<<16;                     // Turn off GPIOC0
	}

	for( i = 0; i < 5; i++ )
	{
		GPIOD->BSHR = 1 | (1<<4);
		GPIOC->BSHR = 1;
		Delay_Ms( 250 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIOD0 + D4
		GPIOC->BSHR = 1<<16;                     // Turn off GPIOC0
		Delay_Ms( 20 );
		count++;
	}

	// Exit bootloader after 5 blinks.

	// Note we have to do this if we ended up in the bootloader because
	// the main system booted us here.  If you don't care, you don't need
	// to turn OBTKEYR back off.
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	FLASH->BOOT_MODEKEYR = FLASH_KEY1;
	FLASH->BOOT_MODEKEYR = FLASH_KEY2;
	FLASH->STATR = 0; // 1<<14 is zero, so, boot user code.
	FLASH->CTLR = CR_LOCK_Set;

	PFIC->SCTLR = 1<<31;
	while(1);
}
