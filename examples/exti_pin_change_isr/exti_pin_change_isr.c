// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

volatile uint32_t count;

// This code uses fast interrupts, but be warned, in this mode your hardware stack 
// is only 2 interrupt calls deep!!!
//
// When you execute code here, from RAM, the latency is between 310 and 480ns.
// From RAM is using  __attribute__((section(".srodata")));
// Flash starts at 340ns but seems about the same otherwise, bar the fact it prevents
// any other apps from running.
void EXTI7_0_IRQHandler( void ) __attribute__((naked));
void EXTI7_0_IRQHandler( void ) 
{
	// Flash just a little blip.
	GPIOC->BSHR = 2;
	GPIOC->BSHR = (2<<16);

	// Acknowledge the interrupt
	EXTI->INTFR = 1<<3;

	// Return out of function.
	asm volatile( "mret" );
}

int main()
{
	SystemInit48HSI();

	// Enable GPIOs
	RCC->APB2PCENR = RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;

	// GPIO D3 for input pin change.
	GPIOD->CFGLR =
		(GPIO_CNF_IN_PUPD)<<(4*1) |  // Keep SWIO enabled.
		(GPIO_SPEED_IN | GPIO_CNF_IN_PUPD)<<(4*3);  //PD4 = GPIOD IN

	// GPIO C0 Push-Pull (our output) 
	GPIOC->CFGLR =
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1);

	// Ugh this is tricky.
	// This is how we set (INTSYSCR) to enable hardware interrupt nesting
	// and hardware stack. BUT this means we can only stack intterrupts 2 deep.
	//
	// Note: If you don't do this, you will need to set your functions to be
	// __attribute__((interrupt)) instead of  __attribute__((naked))  with an
	// mret.
	asm volatile( "addi t1,x0, 3\ncsrrw x0, 0x804, t1\n" : : :  "t1" );


	// Configure the IO as an interrupt.
	AFIO->EXTICR = 3<<(3*2); //PORTD.3 (3 out front says PORTD, 3 in back says 3)
	EXTI->INTENR = 1<<3; // Enable EXT3
	EXTI->RTENR = 1<<3;  // Rising edge trigger

	// enable interrupt
	NVIC_EnableIRQ( EXTI7_0_IRQn );

	while(1)
	{
		// PC1 constantly toggles, but do realy cursed instructons ;)
		// this is so we have something we can "break" from
		GPIOC->BSHR = 1;
		if( ((uint32_t*)count++) )
			GPIOC->BSHR = (1<<16);
		if( count == 100 ) count = 0;
	}
}

