#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK
#include "ch32v003fun.h"


#define CH32V003_SPI_SPEED_HZ 1000000
#define CH32V003_SPI_DIRECTION_2LINE_TXRX
#define CH32V003_SPI_CLK_MODE_POL0_PHA0			//leading = rising		trailing = falling		sample on leading		default if you're unsure
#define CH32V003_SPI_NSS_SOFTWARE_ANY_MANUAL	// toggle manually!
#define CH32V003_SPI_IMPLEMENTATION
#include "../../extralibs/ch32v003_SPI.h"
#include "nrf24l01.h"

/*start of low level functions, specific to the mcu and compiler*/

/*delay in miliseconds*/
void delay_function(uint32_t duration_ms)
{
	Delay_Ms(duration_ms);
}

/*contains all SPI configuations, such as pins and control registers*/
/*SPI control: master, interrupts disabled, clock polarity low when idle, clock phase falling edge, clock up tp 1 MHz*/
void SPI_Initializer()
{
	SPI_init();
	SPI_begin_8();
}

/*contains all CSN and CE pins gpio configurations, including setting them as gpio outputs and turning SPI off and CE '1'*/
void pinout_Initializer()
{
	// CSN on PC0
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
	// CSN high
	GPIOC->BSHR = (1<<0);
	// CE on PC4
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
	// CE HIGH
	GPIOC->BSHR = (1<<4);
}

/*CSN pin manipulation to high or low (SPI on or off)*/
void nrf24_SPI(uint8_t input)
{
	if (input > 0) {
		GPIOC->BSHR = (1<<(0+0));
	}
	else {
		GPIOC->BSHR = (1<<(16+0));
	}
}

/*1 byte SPI shift register send and receive routine*/
uint8_t SPI_send_command(uint8_t command)
{
	return SPI_transfer_8(command);
}

/*CE pin maniplation to high or low*/
void nrf24_CE(uint8_t input)
{
	if (input > 0) {
		GPIOC->BSHR = (1<<(0+4));
	}
	else {
		GPIOC->BSHR = (1<<(16+4));
	}
}
