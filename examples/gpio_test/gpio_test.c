// Basic GPIO and ADC Example using modern funPinMode / funDigitalWrite / etc... modes.
//
// Modes (Assume processor VCC = 3.3V)
//  Push/Pull: 
//   * ADC Works. 
//   * Source 50mA - "1"
//   * Sink   52mA - "0"
//  Open Drain (PC1, 2, 5, 6)
//   * If unset (open) if 5V is applied, no current flows.
//   * Only doesn't draw power if set to "float"
//  Open Drain (Other pins)
//   * Very high Z - "1"
//   * Sink   52mA - "9"
//   * If 5V is applied, pin sinks 20mA (or 86mA if driven low)
//  Analog
//   * No sinking or sourcing
//   * No analog pins are 5V tolerant.
//   * If 5V is applied, pin sinks 20mA.
//  PU/PD
//   * If 5V is applied, pin sinks 20mA.
//   * If set to high, pin sources 76uA from GND.
//   * If set to low, pin sinks 76uA from 3.3v.
//   * If within 0...3.3V, pretend it has 45k Ohms.

#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PD2, GPIO_CFGLR_OUT_50Mhz_PP );
	funPinMode( PC4, GPIO_CFGLR_OUT_50Mhz_OD ); // PC4 = Not 5V tolerant.
	funPinMode( PC1, GPIO_CFGLR_OUT_50Mhz_OD );
	funPinMode( PA1, GPIO_CFGLR_IN_ANALOG );
	funPinMode( PA2, GPIO_CFGLR_IN_PUPD );

	// Enable ADC
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
	
	// JSQ1 = Channel 0 (PA2)
	// JSQ2 = Channel 1 (PA1)
	// JSQ3 = Channel 2 (PC4)
	// JSQ3 = Channel 3 (PD2)
	// 4 Total channels
	ADC1->ISQR = ((0)<<0) | ((1)<<5) | ((2)<<10) | ((3)<<15) | (3<<20);

	// 0:7 => 3/9/15/30/43/57/73/241 cycles
	ADC1->SAMPTR2 |= (7<<(3*0)) | (7<<(3*1)) | (7<<(3*2)) | (7<<(3*3));
		
	// turn on ADC and set rule group to sw trig
	ADC1->CTLR2 = ADC_ADON | ADC_JEXTSEL;

	ADC1->CTLR1 = ADC_SCAN;
	
	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);


	while(1)
	{
		ADC1->CTLR2 |= ADC_JSWSTART;
		while(!(ADC1->STATR & ADC_JEOC));
		printf( "0: %d %d %d %d\n", (int)ADC1->IDATAR1, (int)ADC1->IDATAR2, (int)ADC1->IDATAR3, (int)ADC1->IDATAR4 );
		funDigitalWrite( PD2, 1 );
		funDigitalWrite( PC4, 1 );
		funDigitalWrite( PC1, 1 ); // 5v tolerant, no ADC
		funDigitalWrite( PA1, 1 );
		funDigitalWrite( PA2, 1 );
		Delay_Ms( 1000 );

		ADC1->CTLR2 |= ADC_JSWSTART;
		while(!(ADC1->STATR & ADC_JEOC));
		printf( "1: %d %d %d %d\n", (int)ADC1->IDATAR1, (int)ADC1->IDATAR2, (int)ADC1->IDATAR3, (int)ADC1->IDATAR4 );
		funDigitalWrite( PD2, 0 );
		funDigitalWrite( PC4, 0 );
		funDigitalWrite( PC1, 0 ); // 5v tolerant, no ADC
		funDigitalWrite( PA1, 0 );
		funDigitalWrite( PA2, 0 );
		Delay_Ms( 1000 );
	}
}

