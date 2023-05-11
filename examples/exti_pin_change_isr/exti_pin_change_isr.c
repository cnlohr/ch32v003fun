// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

#define APB_CLOCK SYSTEM_CORE_CLOCK

/*
 This code uses fast interrupts, but be warned, in this mode your hardware stack 
 is only 2 interrupt calls deep!!!

 When you execute code here, from RAM, the latency is between 310 and 480ns.
 From RAM is using  __attribute__((section(".srodata")));

Chart is in Cycles Spent @ 48MHz

|   __attribute__ |  HPE ON  |  HPE OFF |
| ((interrupt))   |   29  |   28  |
| ((section(".srodata"))) and ((interrupt)) | 28 | 23 |
| ((naked)) | 23 | 21 |
| ((section(".srodata"))) and ((naked)) | 15 | 16| 

  These were done with an empty (nop) loop in main.

  HPE ON  = 0x804 = 3
  HPE OFF = 0x804 = 0

  Bog-standard interrupt test with variance. I.e.
	__attribute__((interrupt)) with cursed code in main loop.

  Variance tests: 27-30 cycles
  
  Which will manifest as interurpt jitter.
*/

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
	// Flash just a little blip.
	GPIOC->BSHR = 2;
	GPIOC->BSHR = (2<<16);

	// Acknowledge the interrupt
	EXTI->INTFR = 1<<3;
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
	// This feature is called "HPE"
	//
	// Note: If you don't do this, you will need to set your functions to be
	// __attribute__((interrupt)) instead of  __attribute__((naked))  with an
	// mret.
	//
	// PLEASE BE CAREFUL WHEN DOING THIS THOUGH.  There are a number of things
	// you should know with HPE.  The issue is that HPE doesn't preserve s0,
	// and s1. You should review the following material before using HPE.
	//   https://github.com/cnlohr/ch32v003fun/issues/90
	//   https://www.reddit.com/r/RISCV/comments/126262j/notes_on_wch_fast_interrupts/
	//   https://www.eevblog.com/forum/microcontrollers/bizarre-problem-on-ch32v003-with-systick-isr-corrupting-uart-tx-data
	asm volatile( "addi t1,x0, 3\ncsrrw x0, 0x804, t1\n" : : :  "t1" );

	// Configure the IO as an interrupt.
	AFIO->EXTICR = 3<<(3*2); //PORTD.3 (3 out front says PORTD, 3 in back says 3)
	EXTI->INTENR = 1<<3; // Enable EXT3
	EXTI->RTENR = 1<<3;  // Rising edge trigger

	// enable interrupt
	NVIC_EnableIRQ( EXTI7_0_IRQn );

	while(1)
	{
		//GPIOC->BSHR = 1;
		//if( ((uint32_t*)count++) )
		//	GPIOC->BSHR = (1<<16);
		//if( count == 100 ) count = 0;
		asm volatile( "nop" );
	}
}

