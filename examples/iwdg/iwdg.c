// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

static void iwdg_setup(uint16_t reload_val, uint8_t prescaler) {
	IWDG->CTLR = 0x5555;
	IWDG->PSCR = prescaler;

	IWDG->CTLR = 0x5555;
	IWDG->RLDR = reload_val & 0xfff;

	IWDG->CTLR = 0xCCCC;
}

static void iwdg_feed() {
	IWDG->CTLR = 0xAAAA;
}

static void gpios_on() {
	GPIOD->BSHR = 1 | (1<<4);
	GPIOC->BSHR = 1;
}

static void gpios_off() {
	GPIOD->BSHR = (1<<16) | (1<<(16+4));
	GPIOC->BSHR = (1<<16);
}

int main()
{
	SystemInit48HSI();

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

	gpios_on();
	Delay_Ms( 1500 );
	gpios_off();
	Delay_Ms( 1000 );

	// set up watchdog to about 4 s
	iwdg_setup(0xfff, IWDG_Prescaler_128);

	gpios_on();
	Delay_Ms( 3000 );
	gpios_off();

	// feed the watch dog. Now there should be about 8 blinks
	iwdg_feed();

	while(1)
	{
		gpios_on();
		Delay_Ms( 250 );
		gpios_off();
		Delay_Ms( 250 );
	}
}

