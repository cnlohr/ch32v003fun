#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>

void set_trim(uint32_t trim)
{
	uint32_t regtemp;

	regtemp = RCC->CTLR & ~RCC_HSITRIM;
	regtemp |= trim<<3;
	RCC->CTLR = regtemp;
}


int main()
{
	uint32_t trim, regtemp;
	uint8_t key;

	SystemInit48HSI();
        SetupUART( UART_BRR );

	Delay_Ms(50);
	printf("\r\r\n\nHSITRIM example.\r\n");

	regtemp = RCC->CTLR;
	printf("RCC->CTLR: 0x%08lX : ", regtemp);
	if (regtemp & RCC_PLLRDY) printf("RCC_PLLRDY ");
	if (regtemp & RCC_PLLON)  printf("RCC_PLLON ");
	if (regtemp & RCC_CSSON)  printf("RCC_CSSON ");
	if (regtemp & RCC_HSEBYP) printf("RCC_HSEBYP ");
	if (regtemp & RCC_HSERDY) printf("RCC_HSERDY ");
	if (regtemp & RCC_HSEON)  printf("RCC_HSEON ");
	if (regtemp & RCC_HSIRDY) printf("RCC_HSIRDY ");
	if (regtemp & RCC_HSION)  printf("RCC_HSION ");
	printf("\r\n");
	printf("HSICAL: 0x%02lX\r\n", (regtemp & RCC_HSICAL) >> 8);
	trim = (regtemp & RCC_HSITRIM) >> 3;
	printf("HSITRIM: 0x%02lX\r\n", trim);

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

        // PC4 is T1CH4, 50MHz Output PP CNF = 10: Mux PP, MODE = 11: Out 50MHz
        GPIOC->CFGLR &= ~(GPIO_CFGLR_MODE4 | GPIO_CFGLR_CNF4);
        GPIOC->CFGLR |= GPIO_CFGLR_CNF4_1 | GPIO_CFGLR_MODE4_0 | GPIO_CFGLR_MODE4_1;

	printf("Enabling HSI signal with MCO function on PC4\r\n");
	regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_HSI;
	RCC->CFGR0 = regtemp;

	// Setup UART RX pin SetupUART() does most of the work.  ###beware### if you use DebugPrintf, this will break!
	// Input, GPIO D6, with AutoFunction
        GPIOD->CFGLR &= ~(0xf<<(4*6));
        GPIOD->CFGLR |= (GPIO_CNF_IN_FLOATING)<<(4*6);
	USART1->CTLR1 |= USART_CTLR1_RE;

	printf("\r\nPress ',' to decrease HSITRIM.  Press '.' to increase HSITRIM.\r\n");
	while(1){
		if(USART1->STATR & USART_STATR_RXNE){
			key = (uint8_t)USART1->DATAR;
			if(key == '.'){
				if(trim < 0x1f) trim++;
				printf("Setting HSITRIM to: 0x%02lX\r\n", trim);
				set_trim(trim);
			}
			if(key == ','){
				if(trim > 0x00) trim--;
				printf("Setting HSITRIM to: 0x%02lX\r\n", trim);
				set_trim(trim);
			}
		}
	}
}
