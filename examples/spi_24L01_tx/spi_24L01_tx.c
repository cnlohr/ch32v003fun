/*
 * Example for 24L01+ over SPI, using https://github.com/ebrezadev/nRF24L01-C-Driver
 * 04-26-2023 recallmenot 
 */

#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "nrf24l01.h"



#define TIME_GAP 1000
uint8_t ascending_number = 0x00;
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

// led is PD4 to LED1 on board, which is (-)
inline void led_on() {
	GPIOD->BSHR = 1<<(16+4);
}

inline void led_off() {
	GPIOD->BSHR = 1<<4;
}



//######### TX fn

uint8_t sendnumber() {
	return nrf24_transmit(&ascending_number, 1, ACK_MODE);
}

// function prototype (declaration), definition in "ch32v003fun.c"
int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);

uint8_t sendstr() {
	mini_snprintf(txt, sizeof(txt), "Hello, %u", ascending_number);
	printf("\n\rsending %s\n\r", txt);
	return nrf24_transmit((uint8_t*)txt, 16, ACK_MODE);
}

void send() {
	// to switch between sending an uint8_t and a 16-byte-char-array, just uncomment one of these two:
	//uint8_t tx_cmd_status = sendnumber();
	uint8_t tx_cmd_status = sendstr();
	switch (tx_cmd_status) {
		case TRANSMIT_BEGIN:
		led_on();
			printf("***		sending package\n\r");
			break;
		case TRANSMIT_FAIL:
			printf("EEE		unable to send package\n\r");
			break;
	}

	Delay_Ms(50);					// give the nRF some time to send
	print_debug();

	switch (nrf24_transmit_status()) {
		case TRANSMIT_DONE:
			printf("*OK		sent: %u\n\r", ascending_number);
		led_off();
			break;
		case TRANSMIT_FAILED:
			printf("EEE		no ACK received!!\n\r");
			break;
		case TRANSMIT_IN_PROGRESS:
			printf("EEE		still transmitting???\n\r");
			break;
	}
}



//######### MAIN


int main()
{
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );

	printf("\r\r\n\nspi_24L01_TX\n\r");

	printf("initializing radio as TX...");
	nrf24_device(TRANSMITTER, RESET);
	nrf24_rf_power(18);						//default TX power is -6dB, pretty strong, reduce to -18dBm for one room
	printf("done.\n\r");

	print_debug();

	// GPIO D4 Push-Pull for foreground blink
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	Delay_Ms(1000);

	printf("looping...\n\r");
	while(1)
	{
		Delay_Ms( TIME_GAP );
		send();

		ascending_number++;

		printf("***		next number: %u\n\r", ascending_number);
	}
}
