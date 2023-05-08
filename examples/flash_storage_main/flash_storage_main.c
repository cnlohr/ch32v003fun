// Could be defined here, or in the processor defines.
#include <stdint.h>
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"
#include <stdio.h>
#include "ch32v003_flash.h"



uint16_t count;
//uint32_t nonvolatile_var_addr;
//uint32_t nonvolatile_start_addr;
#define NONVOLATILE_START_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(0)
#define NONVLOATILE_VAR_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(10)



int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	Delay_Ms(5000);

	printf("\r\nnonvolatile storage testing\r\n");
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	flash_set_latency();
	//nonvolatile_start_addr = flash_calcualte_nonvolatile_addr(0);
	//nonvolatile_var_addr = flash_calcualte_nonvolatile_addr(10);
	printf("FLASH_LENGTH_OVERRIDE is        %u\r\n", (uint16_t)(uintptr_t)FLASH_LENGTH_OVERRIDE);
	//printf("non-volatile start address is   %lu\r\n", nonvolatile_start_addr);
	//printf("non-volatile var address is     %lu\r\n", nonvolatile_var_addr);
	printf("non-volatile start address is   %lu\r\n", NONVOLATILE_START_ADDR);
	printf("non-volatile var address is     %lu\r\n", NONVLOATILE_VAR_ADDR);

	while(1)
	{
		GPIOD->BSHR = (1<<(16+4)); // LED on
		//count = flash_get_16(nonvolatile_var_addr);
		count = flash_get_16(NONVLOATILE_VAR_ADDR);
		printf("   memory contained value %u\r\n", count);
		Delay_Ms(250);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(9750);

		count--;
		flash_unlock();
		printf("memory unlocked\r\n");
		//flash_erase_64b(nonvolatile_start_addr);
		flash_erase_64b(NONVOLATILE_START_ADDR);
		printf("memory erased\r\n");
		//flash_program_16(nonvolatile_var_addr, count);
		flash_program_16(NONVLOATILE_VAR_ADDR, count);
		printf("memory written\r\n");
		flash_lock();
		printf("memory locked\r\n");
	}
}
