// Could be defined here, or in the processor defines.
#include <stdint.h>
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"
#include <stdio.h>
#include "../flash_storage_main/ch32v003_flash.h"



uint16_t count;



//######### debug fn

void uint16_to_binary_string(uint16_t value, char* output, int len) {
	for (int i = 0; i < len; i++) {
			output[len - i - 1] = (value & 1) ? '1' : '0';
			value >>= 1;
	}
	output[len] = '\0';
}

void uint8_to_binary_string(uint8_t value, char* output, int len) {
	for (int i = 0; i < len; i++) {
			output[len - i - 1] = (value & 1) ? '1' : '0';
			value >>= 1;
	}
	output[len] = '\0';
}

void print_reg_8(char* name, uint8_t val) {
	char str[9];
	uint8_to_binary_string(val, str, 8);
	printf("   %s register: %s\n\r", name, str);
}

void print_reg_16(char* name, uint16_t val) {
	char str[17];
	uint16_to_binary_string(val, str, 16);
	printf("   %s register: %s\n\r", name, str);
}

void print_debug_8() {
	print_reg_8("OB USER ", flash_OB_get_USER());
	print_reg_8("OB RDPR ", flash_OB_get_RDPR());
	print_reg_8("OB WRPR1", flash_OB_get_WRPR1());
	print_reg_8("OB WRPR0", flash_OB_get_WRPR0());
	print_reg_8("OB DATA1", flash_OB_get_DATA1());
	print_reg_8("OB DATA0", flash_OB_get_DATA0());
}

void print_debug_16() {
	print_reg_16("OB USER ", OB->USER);
	print_reg_16("OB RDPR ", OB->RDPR);
	print_reg_16("OB WRPR1", OB->WRPR1);
	print_reg_16("OB WRPR0", OB->WRPR1);
	print_reg_16("OB DATA1", OB->Data1);
	print_reg_16("OB DATA0", OB->Data0);
}



//######### main

int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	Delay_Ms(5000);

	printf("\r\nnonvolatile storage testing: option bytes\r\n");
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	flash_set_latency();

	while(1)
	{
		GPIOD->BSHR = (1<<(16+4)); // LED on
		print_debug_8();
		count = flash_OB_get_DATA_16();
		printf("   memory contained value %u\r\n", count);
		Delay_Ms(250);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(9750);

		count--;
		flash_unlock();
		flash_OB_unlock();
		printf("memory unlocked\r\n");
		flash_OB_write_data_16(count);
		printf("memory written\r\n");
		flash_lock();
		printf("memory locked\r\n");
	}
}
