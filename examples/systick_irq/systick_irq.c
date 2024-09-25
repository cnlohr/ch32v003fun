/*
 * Example for using SysTick with IRQs
 * 03-25-2023 E. Brombaugh
 * 05-12-2023 C. Lohr (Modified to reflect updated sysclk)
 * 09-25-2024 ADBeta (Minor updates to main loop, comments and added
 *                    convinient macro function)
 */

#include "ch32v003fun.h"
#include <stdio.h>

#define SYSTICK_CMP_1_MILLISECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 1000)

// Simple macro function to give a sensible function to call
#define millis() (systick_millis)

// Incrimented in the SysTick IRQ - in this example once per millisecond
volatile uint32_t systick_millis;

/*
 * Initialises the SysTick to trigger an IRQ with auto-reload, using HCLK/1 as
 * its clock source
 */
void systick_init(void)
{
	// Reset any pre-existing configuration
	SysTick->CTLR = 0x0000;
	
	// Set the compare register to trigger once per millisecond
	SysTick->CMP = SYSTICK_CMP_1_MILLISECOND - 1;

	// Reset the Count Register, and the global millis counter to 0
	SysTick->CNT = 0x00000000;
	systick_millis = 0x00000000;
	
	// Set the SysTick Configuration
	SysTick->CTLR |= SYSTICK_CTLR_STE   |  // Enable Counter
		             SYSTICK_CTLR_STIE  |  // Enable Interrupts
		             SYSTICK_CTLR_STCLK ;  // Set Clock Source to HCLK/1
	
	// Enable the SysTick IRQ
	NVIC_EnableIRQ(SysTicK_IRQn);
}

/*
 * SysTick ISR - must be lightweight to prevent the CPU from bogging down.
 * Incriments Compare Register and systick_millis when triggered (every 1ms)
 * NOTE: the `__attribute__((interrupt))` attribute is very important
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	// Incriment the Compare Register for the next trigger
	// If more than this number of ticks elapse before the tirgger is reset,
	// you may miss your next interrupt trigger
	// (Make sure the IQR is lightweight and CMP value is reasonable)
	SysTick->CMP += SYSTICK_CMP_1_MILLISECOND;

	// Clear the trigger state for the next IRQ
	SysTick->SR = 0x00000000;

	// Incriment the milliseconds count
	systick_millis++;
}


int main(void)
{
	SystemInit();
	Delay_Ms(100);

	printf("\n\nsystick_irq example\n");

	// Initialise the IRQ 
	printf("initializing systick...");
	systick_init();
	printf("done.\n");
	
	// Enable GPIOs for demonstation
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

	
	printf("Beginning Loop...\n");
	while(1)
	{
		// Toggle the GPIO Pins with a delay - total delay will be 500ms
		uint32_t start_millis = millis();
		// On
		GPIOD->BSHR = 1 | (1<<4);
		GPIOC->BSHR = 1;
		Delay_Ms(250);
		// Off
		GPIOD->BSHR = (1<<16) | (1<<(16+4));
		GPIOC->BSHR = (1<<16);
		Delay_Ms(250);
		uint32_t end_millis = millis();
		
		// NOTE: Due to the time it takes for printf(), the Current Millis will
		// incirment more than 500 per loop
		printf("\nMilliseconds taken:\t%lu\n", end_millis - start_millis);
		printf("Current Milliseconds:\t%lu\n", millis());
		printf("SysTick->CNT:\t\t%lu\n", SysTick->CNT);
	}
}
