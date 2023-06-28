#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	uint32_t count, regtemp;

	SystemInit();

	Delay_Ms(50);
	printf("\r\r\n\nTesting MCO output options.\r\n");
	count=0;

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

        // PC4 is T1CH4, 50MHz Output PP CNF = 10: Mux PP, MODE = 11: Out 50MHz
        GPIOC->CFGLR &= ~(GPIO_CFGLR_MODE4 | GPIO_CFGLR_CNF4);
        GPIOC->CFGLR |= GPIO_CFGLR_CNF4_1 | GPIO_CFGLR_MODE4_0 | GPIO_CFGLR_MODE4_1;

	while(1)
	{
		switch(count)
		{
			case 0:
				printf("\r\nNo signal on MCO\r\n");
				regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO);
				printf("CFGR0 going from %08lX to %08lX\r\n", RCC->CFGR0, regtemp);
				RCC->CFGR0 = regtemp;
				count++;
				break;
			case 1:
				printf("\r\nSYSCLK signal on MCO\r\n");
				regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_SYSCLK;
				printf("CFGR0 going from %08lX to %08lX\r\n", RCC->CFGR0, regtemp);
				RCC->CFGR0 = regtemp;
				count++;
				break;
			case 2:
				printf("\r\nHSI signal on MCO\r\n");
				regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_HSI;
				printf("CFGR0 going from %08lX to %08lX\r\n", RCC->CFGR0, regtemp);
				RCC->CFGR0 = regtemp;
				count++;
				break;
			case 3:
				printf("\r\nHSE signal on MCO\r\n");
				regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_HSE;
				printf("CFGR0 going from %08lX to %08lX\r\n", RCC->CFGR0, regtemp);
				RCC->CFGR0 = regtemp;
				count++;
				break;
			case 4:
				printf("\r\nPLLCLK signal on MCO\r\n");
				regtemp = (RCC->CFGR0 & ~RCC_CFGR0_MCO) | RCC_CFGR0_MCO_PLL;
				printf("CFGR0 going from %08lX to %08lX\r\n", RCC->CFGR0, regtemp);
				RCC->CFGR0 = regtemp;
				count=0;
				break;
		}
		Delay_Ms(5000);
	}
}
