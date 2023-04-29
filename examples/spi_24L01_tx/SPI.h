#ifndef SPI_H
#define SPI_H

#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


void SPI_poweron();

void kill_interrrupts();
void restore_interrupts();

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


void SPI_NSS_software_high();
void SPI_NSS_software_low();

void SPI_init(
		uint32_t speedHz,
		enum SPI_clk_modes clockMode,
		enum SPI_data_directions dataDirection,
		enum SPI_NSS_options NSSmode);
void SPI_begin_8();
void SPI_begin_16();
void SPI_end();


void SPI_wait_TX_complete();
uint8_t SPI_is_RX_empty();
void SPI_wait_RX_available();


void SPI_write_8(uint8_t data);
void SPI_write_16(uint16_t data);
uint8_t SPI_read_8();
uint16_t SPI_read_16();


uint8_t SPI_transfer_8(uint8_t data);
uint8_t SPI_transfer_16(uint16_t data);


void SPI_poweroff();


#endif // SPI_H
