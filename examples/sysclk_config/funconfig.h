#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define FUNCONF_USE_HSE 1  			// external crystal on PA1 PA2
#define FUNCONF_USE_HSI 0    		// internal 24MHz clock oscillator
#define FUNCONF_USE_PLL 1			// use PLL x2
#define FUNCONF_HSE_BYPASS 0 		// bypass the HSE when using an external clock source
									// requires enabled HSE
#define FUNCONF_USE_CLK_SEC	1		// clock security system
#define CH32V003        1

#endif // _FUNCONFIG_Hma
