/*
 * Example for 24L01+ over SPI, using https://github.com/ebrezadev/nRF24L01-C-Driver
 * 04-26-2023 recallmenot 
 */

// Could be defined here, or in the processor defines.
#include <stdint.h>
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"
#include <stdio.h>
#include "nrf24l01.h"

#define TIME_GAP			300

uint8_t ascending_number = 0;
char txt[16];



// ####### start of main program ###############################################################

// wire PD4 to LED1 on the dev board (-)
void led_on() {
	GPIOD->BSHR |= 1<<(16+4);
}

void led_off() {
	GPIOD->BSHR |= 1<<4;
}



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
			printf("***   RX success, received: %u\n\r", ascending_number);
			//printf("***   RX success, received: %s\n\r", txt);
			led_off();
			break;
	}
	Delay_Ms(TIME_GAP);
}

int main()
{
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	Delay_Ms( 100 );

	// GPIO D0 Push-Pull for RX notification
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	printf("\r\r\n\nspi_24L01_RX\n\r");

	printf("initializing radio as RX...");
	nrf24_device(RECEIVER, RESET);
	printf("done.\n\r");

	printf("looping...\n\r");
	while(1)
	{
		receive();
	}
}
