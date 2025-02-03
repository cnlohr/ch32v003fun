// Basic GPIO and Injection-Mode ADC Example using modern funPinMode / funDigitalWrite / etc... modes.
//
// The ADC has two modes: Rule and Injection.
//  Injection allows you to read up to 4 ADCs in rapid succession.
//  But there is no rule at the end for checking completion of conversion... So it's usually only good for continuous monitoring.
//
// This demo should alternate between 0 and 1023 for the first 3 channels, and the 4th channel will go a little bit up/down but only because of latent capacitance.

#include "ch32fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();

	// You can use pull-up/pull-down in conjuction with ADC.
	funPinMode( PA1, GPIO_CFGLR_IN_PUPD );
	funPinMode( PA2, GPIO_CFGLR_IN_PUPD );
	funPinMode( PC4, GPIO_CFGLR_IN_PUPD );
	funPinMode( PD2, GPIO_CFGLR_IN_ANALOG );

	// Enable ADC
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// Setup channel conversion situation
	ADC1->ISQR=
		ADC_JL_0 | ADC_JL_1 |     // 4 Total channels
		ADC_JSQ4_1 | ADC_JSQ4_0 | // Channel 3 (PD2)
		ADC_JSQ3_1 |              // Channel 2 (PC4)
		ADC_JSQ2_0 |              // Channel 1 (PA1)
		0;                        // Channel 0 (PA2)

	// Once we read the analog values will be populated in the following:
	// ADC1->JSQ1 = Channel 0 (PA2)
	// ADC1->JSQ2 = Channel 1 (PA1)
	// ADC1->JSQ3 = Channel 2 (PC4)
	// ADC1->JSQ3 = Channel 3 (PD2)

	// Setup sampling time
	// 0:7 => 3/9/15/30/43/57/73/241 cycles
	ADC1->SAMPTR2 |= 
				ADC_SMP0_0 | ADC_SMP0_1 | ADC_SMP0_2 | \
				ADC_SMP1_0 | ADC_SMP1_1 | ADC_SMP1_2 | \
				ADC_SMP2_0 | ADC_SMP2_1 | ADC_SMP2_2 | \
				ADC_SMP3_1 | ADC_SMP3_1 | ADC_SMP3_2;

	// turn on ADC and set rule group to sw trig
	// Additionally ADC_CONT says just continuously read the values.
	ADC1->CTLR2 = ADC_ADON | ADC_JEXTSEL | ADC_CONT;

	ADC1->CTLR1 = ADC_SCAN;
	
	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);

	// Tell the ADC to start converting, continously.
	ADC1->CTLR2 |= ADC_JSWSTART;

	while(1)
	{
		funDigitalWrite( PD2, FUN_HIGH );
		funDigitalWrite( PC4, FUN_HIGH );
		funDigitalWrite( PA1, FUN_HIGH );
		funDigitalWrite( PA2, FUN_HIGH );

		Delay_Us(50); // Give the pins about 50us to get some ADC readings in.

		printf( "1: %d %d %d %d\n", (int)ADC1->IDATAR1, (int)ADC1->IDATAR2, (int)ADC1->IDATAR3, (int)ADC1->IDATAR4 );

		Delay_Ms(1000);

		funDigitalWrite( PD2, FUN_LOW );
		funDigitalWrite( PC4, FUN_LOW );
		funDigitalWrite( PA1, FUN_LOW );
		funDigitalWrite( PA2, FUN_LOW );

		Delay_Us(50); // Give the pins about 50us to get some ADC readings in.

		printf( "0: %d %d %d %d\n", (int)ADC1->IDATAR1, (int)ADC1->IDATAR2, (int)ADC1->IDATAR3, (int)ADC1->IDATAR4 );

		Delay_Ms(1000);
	}
}

