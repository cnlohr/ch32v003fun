// you'll need to #define CH32V003_SPI_IMPLEMENTATION in the .c files that use this library.

// include guards
#ifndef CH32V003_SPI_H
#define CH32V003_SPI_H

// includes
#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"



//######### SPI configuration states, use these for init()
// SPI peripheral config options
/* clock polarity and phase
CPOL	leading		trailing	CPHA	sample on
0		rising		falling		0		leading
0		rising		falling		1		trailing
1		falling		rising		0		leading
1		falling		rising		1		trailing
*/
enum SPI_clk_modes{
	SPI_clk_mode_pol0_pha0_default,
	SPI_clk_mode_pol0_pha1,
	SPI_clk_mode_pol1_pha0,
	SPI_clk_mode_pol1_pha1,
};
enum SPI_data_directions {
	SPI_data_direction_2line_TxRx,				// RX + TX			2-line bidirectional
	SPI_data_direction_2line_Rx,				// RX 				2-line bidirectional
	SPI_data_direction_1line_Rx,				// RX				1-line unidirectional
	SPI_data_direction_1line_Tx,				// TX				1-line unidirectional
};
enum SPI_NSS_options {
	SPI_NSS_hardware_PC0_default,				// _NSS
	SPI_NSS_hardware_PC1,						// NSS but clashes with I2C SDA
	SPI_NSS_software_PC3,
	SPI_NSS_software_PC4,
	SPI_NSS_software_any_manual,
};



//######### public function declarations, use these!
// initialize and configure the SPI peripheral
void SPI_init(
		uint32_t speedHz,
		enum SPI_clk_modes clockMode,
		enum SPI_data_directions dataDirection,
		enum SPI_NSS_options NSSmode);

// establish / end a connection to the SPI device
void SPI_begin_8();
void SPI_begin_16();
void SPI_end();

// manually set the NSS pin high / low
void SPI_NSS_software_high();
void SPI_NSS_software_low();

// read / write the SPI device
// these commands are raw, you'll have to consider all other steps in SPI_transfer!
uint8_t SPI_read_8();
uint16_t SPI_read_16();
void SPI_write_8(uint8_t data);
void SPI_write_16(uint16_t data);

// send a command and get a response from the SPI device
// you'll use this for most devices
uint8_t SPI_transfer_8(uint8_t data);
uint8_t SPI_transfer_16(uint16_t data);

// SPI peripheral power enable / disable (default off, init() automatically enables)
// send SPI peripheral to sleep
void SPI_poweroff();
// wake SPI peripheral from sleep
void SPI_poweron();

// helper: kill / restore all interrupts on the CH32V003
void kill_interrrupts();
void restore_interrupts();





//############ h-file-only implementation section
// you'll need to #define CH32V003_SPI_IMPLEMENTATION in the .c files that use this library.
//#define CH32V003_SPI_IMPLEMENTATION //enable here so LSP can give you text colors while working on this library, disable for normal use of the library
#ifdef CH32V003_SPI_IMPLEMENTATION



//######### internal function declarations
static inline void SPI_wait_TX_complete();
static inline uint8_t SPI_is_RX_empty();
static inline void SPI_wait_RX_available();

// min and max helper macros
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//######### internal variables
static enum SPI_NSS_options NSS_selected;
static uint16_t EXT1_INTENR_backup;

//#########  public function definitions
void SPI_init(
		uint32_t speedHz,
		enum SPI_clk_modes clockMode,
		enum SPI_data_directions dataDirection,
		enum SPI_NSS_options NSSmode) {
	SPI_poweron();
	
	// reset control register
	SPI1->CTLR1 = 0;

	// determine correct baud rate prescaler
	// log2(fCPU / SPIspeed) - 1
	uint8_t clk_ratio = 48000000 / speedHz;
	uint8_t BR = 0;
	// magic rightshift log2 loop
	while (clk_ratio >>= 1) {
		BR++;
	}
	//BR--;

	// confine baud rate prescaler to possible values
	BR = MAX(0,BR);
	BR = MIN(7,BR);

	// set prescaler
	SPI1->CTLR1 |= SPI_CTLR1_BR & (BR<<3);

	// set clock polarity and phase 
	SPI1->CTLR1 |= (SPI_CTLR1_CPOL | SPI_CTLR1_CPHA) & clockMode;
	
	// configure NSS pin for master mode
	NSS_selected = NSSmode;
	switch (NSSmode) {
		case SPI_NSS_hardware_PC0_default:
			// _NSS (negative slave select) on PC0, 10MHz Output, alt func, push-pull1
			SPI1->CTLR1 |= SPI_NSS_Hard;					// NSS hardware control mode
			GPIOC->CFGLR &= ~(0xf<<(4*0));
			GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*0);
			AFIO->PCFR1 |= GPIO_Remap_SPI1;					// remap NSS (C1) to _NSS (C0)
			SPI1->CTLR2 |= SPI_CTLR2_SSOE;					// pull _NSS high
			break;
		case SPI_NSS_hardware_PC1:
			// NSS (negative slave select) on PC1, 10MHz Output, alt func, push-pull1
			SPI1->CTLR1 |= SPI_NSS_Hard;					// NSS hardware control mode
			GPIOC->CFGLR &= ~(0xf<<(4*1));
			GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);
			SPI1->CTLR2 |= SPI_CTLR2_SSOE;					// pull _NSS high
			break;
		case SPI_NSS_software_PC3:
			SPI1->CTLR1 |= SPI_NSS_Soft;					// SSM NSS software control mode
			GPIOC->CFGLR &= ~(0xf<<(4*3));
			GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);
			
			break;
		case SPI_NSS_software_PC4:
			SPI1->CTLR1 |= SPI_NSS_Soft;					// SSM NSS software control mode
			GPIOC->CFGLR &= ~(0xf<<(4*4));
			GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);
			break;
		case SPI_NSS_software_any_manual:
			SPI1->CTLR1 |= SPI_NSS_Soft;					// SSM NSS software control mode
			break;
	}

	// SCK on PC5, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*5));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

	// CH32V003 is master
	SPI1->CTLR1 |= SPI_Mode_Master;
	
	// set data direction and configure data pins
	switch (dataDirection) {
		case SPI_data_direction_2line_TxRx:
			SPI1->CTLR1 |= SPI_Direction_2Lines_FullDuplex;

			// MOSI on PC6, 10MHz Output, alt func, push-pull
			GPIOC->CFGLR &= ~(0xf<<(4*6));
			GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
			
			// MISO on PC7, 10MHz input, floating
			GPIOC->CFGLR &= ~(0xf<<(4*7));
			GPIOC->CFGLR |= GPIO_CNF_IN_FLOATING<<(4*7);
			break;
		case SPI_data_direction_2line_Rx:
			// slave-only
			break;
		case SPI_data_direction_1line_Rx:
			// slave-only
			break;
		case SPI_data_direction_1line_Tx:
			SPI1->CTLR1 |= SPI_Direction_1Line_Tx;

			// MOSI on PC6, 10MHz Output, alt func, push-pull
			GPIOC->CFGLR &= ~(0xf<<(4*6));
			GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
			
			break;
	}
}

void SPI_begin_8() {
	SPI1->CTLR1 |= SPI_DataSize_8b;					// DFF 16bit data-length enable, writable only when SPE is 0
	SPI1->CTLR1 |= CTLR1_SPE_Set;
}
void SPI_begin_16() {
	SPI1->CTLR1 |= SPI_DataSize_16b;				// DFF 16bit data-length enable, writable only when SPE is 0
	SPI1->CTLR1 |= CTLR1_SPE_Set;
}
void SPI_end() {
	SPI1->CTLR1 &= CTLR1_SPE_Reset;
}

// software NSS output high
void SPI_NSS_software_high() {
	switch (NSS_selected) {
		case SPI_NSS_software_PC3:
			GPIOC->BSHR |= (1<<3);
			break;
		case SPI_NSS_software_PC4:
			GPIOC->BSHR |= (1<<4);
			break;
		default:
			break;
	}
}
// software NSS output low
void SPI_NSS_software_low() {
	switch (NSS_selected) {
		case SPI_NSS_software_PC3:
			GPIOC->BSHR &= ~(1<<(16+3));
			break;
		case SPI_NSS_software_PC4:
			GPIOC->BSHR &= ~(1<<(16+4));
			break;
		default:
			break;
	}
}

uint8_t SPI_read_8() {
	return SPI1->DATAR;
}
uint16_t SPI_read_16() {
	return SPI1->DATAR;
}
void SPI_write_8(uint8_t data) {
	SPI1->DATAR = data;
}
void SPI_write_16(uint16_t data) {
	SPI1->DATAR = data;
}
uint8_t SPI_transfer_8(uint8_t data) {
	SPI_NSS_software_high();
	SPI_write_8(data);
	SPI_wait_TX_complete();
	asm volatile("nop");
	SPI_wait_RX_available();
	SPI_NSS_software_low();
	return SPI_read_8();
}
uint8_t SPI_transfer_16(uint16_t data) {
	SPI_NSS_software_high();
	SPI_write_16(data);
	SPI_wait_TX_complete();
	asm volatile("nop");
	SPI_wait_RX_available();
	SPI_NSS_software_low();
	return SPI_read_16();
}

void SPI_poweroff() {
	SPI_end();
	RCC->APB2PCENR &= ~RCC_APB2Periph_SPI1;
}
void SPI_poweron() {
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;
}

void kill_interrrupts() {
	EXT1_INTENR_backup = EXTI->INTENR;
	// zero the interrupt enable register to disable all interrupts
	EXTI->INTENR = 0;
}
void restore_interrupts() {
	EXTI->INTENR = EXT1_INTENR_backup;
}



//#########  internal function definitions

static void SPI_wait_TX_complete() {
	while(!(SPI1->STATR & SPI_STATR_TXE)) {
		asm volatile("nop");
	}
}
static uint8_t SPI_is_RX_empty() {
	return SPI1->STATR & SPI_STATR_RXNE;
}
static void SPI_wait_RX_available() {
	while(!(SPI1->STATR & SPI_STATR_RXNE)) {
		asm volatile("nop");
	}
}

#endif // CH32V003_SPI_IMPLEMENTATION
#endif // CH32V003_SPI_H
