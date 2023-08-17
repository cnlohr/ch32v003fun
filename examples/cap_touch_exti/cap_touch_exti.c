/* Demo showing how to use eternal interrupts to detect touch.  This allows for
	both low resolution touch (about 6-7 bits for a 6mm x 10mm pad) by using
	internal pull up resistors, as well as high resolution touch (8+ bits per
	pad) with a 560k pull up resistor.

	The current configuration uses:

	PORT C, pin 4 - regular touch pad, no extra components.
	PORT C, pin 5 - high res pad, with pull-up to 3.3v with 560k resistor.
	PORT D, pin 5 - regular touch pad, no extra components.

	Note: Typical count deltas for a 1 cm^2 pad are typically only 100 lsb's.
	So, this method is really limited to boolean "down" functions.
*/

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;
uint32_t endtime;

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
	endtime = SysTick->CNT;
	EXTI->INTFR = 0xffffffff;
}

#define GPIOPortByBase( i )   ((GPIO_TypeDef *)(GPIOA_BASE + 0x0400 * (i)))


// The "port" is:
// 0 for Port A
// 1 for Port B
// 2 for Port C
// 3 for Port D
int MeasureTouch( int portno, int pin, int pu_mode )
{
	uint32_t starttime;
	GPIO_TypeDef * port = GPIOPortByBase( portno );
	uint32_t pinx4 = pin<<2;

	// Mask out just our specific port.  This way we don't interfere with other
	// stuff that may be on this port.
	uint32_t base = port->CFGLR & (~(0xf<<pinx4));

	// Mode for CFGLR when asserted.
	uint32_t setmode =     base | (GPIO_CFGLR_OUT_2Mhz_PP)<<(pinx4);

	// Mode for CFGLR when it drifts.
	uint32_t releasemode = base | (pu_mode)<<(pinx4);

	// Assert pin
	port->CFGLR = setmode;
	port->BSHR = 1<<(pin+16);

	// Setup pin-change-interrupt.  This will trigger when the voltage on the
	// pin rises above the  schmitt trigger threshold.
	AFIO->EXTICR = portno<<(pin*2);
	EXTI->INTENR = 0xffffffff;  // Enable EXT3
	EXTI->RTENR = 0xffffffff;   // Rising edge trigger

	// Tricky, we want the release to happen at an un-aligned address. 
	// This actually doubles our touch sensor resolution.
	asm volatile( ".balign 4; c.nop" );
	port->CFGLR = releasemode;
	starttime = SysTick->CNT;
	endtime = starttime - 1;
	port->BSHR = 1<<(pin);

	// Allow up to 384 cycles for the pin to change.
	#define DELAY8 \
		asm volatile( "c.nop;c.nop;c.nop;c.nop;c.nop;c.nop;c.nop;c.nop;" );
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8
	DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8 DELAY8

	// Optimization: If you did the nop sled in assembly, the interrupt could scoot to the end

	// Disable EXTI
	EXTI->INTENR = 0;

	// Optional assert pin when done to prevent it from noodling around.
	//port->CFGLR = setmode;
	//port->BSHR = 1<<(pin+16);

	return endtime - starttime;
}

int main()
{
	SystemInit();

	// Enable GPIOs and pin-change interrupt.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA |
		RCC_APB2Periph_AFIO;

	// enable pin-change-interrupt.
	NVIC_EnableIRQ( EXTI7_0_IRQn );

	while(1)
	{
		printf( "%d %d %d %d %d %d %d %d\n",
			MeasureTouch( 2, 4, GPIO_CFGLR_IN_PUPD ), // Port C4
			MeasureTouch( 2, 5, GPIO_CFGLR_IN_FLOAT ),  // Port C5, with external pull-up.
			MeasureTouch( 2, 7, GPIO_CFGLR_IN_PUPD ), // Port C7
			MeasureTouch( 3, 2, GPIO_CFGLR_IN_PUPD ), // Port D2
			MeasureTouch( 3, 3, GPIO_CFGLR_IN_PUPD ), // Port D3
			MeasureTouch( 3, 4, GPIO_CFGLR_IN_PUPD ), // Port D4
			MeasureTouch( 3, 5, GPIO_CFGLR_IN_PUPD ), // Port D5
			MeasureTouch( 3, 6, GPIO_CFGLR_IN_PUPD ) // Port D6
			 );
	}
}

