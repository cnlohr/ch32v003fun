#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA8, GPIO_CFGLR_OUT_50Mhz_AF_PP );

	RCC->CFGR0 = RCC_CFGR0_MCO_PLL;

	while(1)
	{
	}
}

