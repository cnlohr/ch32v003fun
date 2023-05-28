/*
 * Example for 24L01+ over SPI, using https://github.com/ebrezadev/nRF24L01-C-Driver
 * 04-26-2023 recallmenot 
 */

#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "nrf24l01.h"



#define TIME_GAP 300
uint8_t ascending_number = 0;
char txt[16];



//######### debug fn

void uint8_to_binary_string(uint8_t value, char* output, int len) {
		for (int i = 0; i < len; i++) {
				output[len - i - 1] = (value & 1) ? '1' : '0';
				value >>= 1;
		}
		output[len] = '\0';
}


void print_reg(char* name, uint8_t addr) {
	char str[9];
	uint8_t REG;
	nrf24_read(addr, &REG, 1, CLOSE);
	uint8_to_binary_string(REG, str, 8);
	printf("				 %s register: %s\n\r", name, str);
}


void print_debug() {
	print_reg("FEATURE      ", FEATURE_ADDRESS);
	print_reg("TX OBSERVE   ", OBSERVE_TX_ADDRESS);
	print_reg("STATUS       ", STATUS_ADDRESS);
	print_reg("RX_PW_P0 ADDR", RX_ADDR_P0_ADDRESS);
	print_reg("TX ADDR      ", TX_ADDR_ADDRESS);
	print_reg("EN_AA        ", EN_AA_ADDRESS);
	print_reg("EN_RXADDR    ", EN_RXADDR_ADDRESS);
}



//######### LED fn

// wire PD4 to LED1 on the dev board (-)
inline void led_on() {
	GPIOD->BSHR = 1<<(16+4);
}

inline void led_off() {
	GPIOD->BSHR = 1<<4;
}



//######### RX fn

uint8_t recvnumber() {
  return nrf24_receive(&ascending_number, 1);
}

uint8_t recvstr() {
  return nrf24_receive((uint8_t*)&txt, 16);
}

void receive() {
	// to switch between sending an uint8_t and a 16-byte-char-array, just uncomment one of these two:
	//uint8_t result = recvnumber();
	uint8_t result = recvstr();
	// also uncomment the corresponding one for case OPERATION_DONE

	//print_debug();
	switch(result) {
		case OPERATION_ERROR:
			printf("EEE   RX operation error\n\r");
			break;
		case RECEIVE_FIFO_EMPTY:
			printf("      RX empty\n\r");
			//printf("      RX empty, last received: %u", ascending_number);
			break;
		case OPERATION_DONE:
			led_on();
			// pick one of these two:
			//printf("***   RX success, received: %u\n\r", ascending_number);
			printf("***   RX success, received: %s\n\r", txt);
			led_off();
			break;
	}
	Delay_Ms(TIME_GAP);
}



//######### MAIN

int main()
{
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	Delay_Ms( 100 );

	printf("\r\r\n\nspi_24L01_RX\n\r");

	printf("initializing radio as RX...");
	nrf24_device(RECEIVER, RESET);
	nrf24_rf_power(18);						//default TX power is -6dB, pretty strong, reduce to -18dBm for one room (ACK = TX)
	//nrf24_automatic_retransmit_setup(RETRANSMIT_DELAY_DEFAULT, 0);
	printf("done.\n\r");

	print_debug();

	// GPIO D0 Push-Pull for RX notification
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	Delay_Ms(1000);

	printf("looping...\n\r");
	while(1)
	{
		receive();
	}
}
