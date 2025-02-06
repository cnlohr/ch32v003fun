// Basic GPIO and Injection-Mode ADC Example using modern funPinMode / funDigitalWrite / etc... modes.
//
// Modes (Assume processor VCC = 3.3V)
//  Push/Pull (PP): 
//   * ADC Works. 
//   * Output 1 (FUN_HIGH): GPIO Sources 50mA
//   * Output 0 (FUN LOW): GPIO Sinks 52mA
//  Open Drain (OD)
//   (PC1, 2, 5, 6, 5V tolerant pins)
//     * Output 1: if 5V is applied, no current flows.
//     * Output 0: Pin is driven low.
//  Open Drain (Other pins)
//   * Very high Z - "1"
//   * Sink   52mA - "0"
//   * If 5V is applied, pin sinks 20mA (or 86mA if driven low)
//  Analog or Float
//   * No sinking or sourcing
//   * Output 0 or 1 has no effect.
//   * No analog pins are 5V tolerant.
//   * If 5V is applied to 5v-tolearnt pin (PC1, 2, 5, 6, 5V tolerant pins) no current flows.
//   * If 5V is applied to other pins, pin sinks 20mA from 5V
//  Pull-up and Pull-down (PU/PD)
//   * Output 1: pin sources 76uA from GND.
//   * Output 0: pin sinks 76uA from 3.3v.
//   * I.e. If you want to have a pull-up, configure PUPD, then set output to 1.
//   * If 5V is applied, pin sinks 20mA.
//   * If within 0...3.3V, pretend it has 45k Ohms.

#include "ch32fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();
	funAnalogInit();

	funPinMode( PA1, GPIO_CFGLR_IN_ANALOG );    // Corresponds to analog in A0
	funPinMode( PA2, GPIO_CFGLR_IN_PUPD );      // Corresponds to analog in A1
	funPinMode( PC4, GPIO_CFGLR_OUT_50Mhz_OD ); // Corresponds to analog in A2
	funPinMode( PD2, GPIO_CFGLR_OUT_50Mhz_PP ); // Corresponds to analog in A3
	funPinMode( PC1, GPIO_CFGLR_OUT_50Mhz_OD ); // PC1 = 5V tolerant (so no ADC)

	while(1)
	{
		// Drive digitals high
		funDigitalWrite( PA1, FUN_HIGH /* = 1 */ );
		funDigitalWrite( PA2, FUN_HIGH );
		funDigitalWrite( PC4, FUN_HIGH );
		funDigitalWrite( PD2, FUN_HIGH );
		funDigitalWrite( PC1, FUN_HIGH ); // 5v tolerant, no ADC

		Delay_Ms(1000);

		printf( "1: %d %d %d %d\n",
			funAnalogRead( ANALOG_0 ),
			funAnalogRead( ANALOG_1 ),
			funAnalogRead( ANALOG_2 ),
			funAnalogRead( ANALOG_3 ) );

		printf( "   %d %d %d %d %d\n\n",
			funDigitalRead( PA1 ),
			funDigitalRead( PA2 ),
			funDigitalRead( PC4 ),
			funDigitalRead( PD2 ),
			funDigitalRead( PC1 ) );

		// Drive digitals low
		funDigitalWrite( PA1, FUN_LOW /* = 0 */ );
		funDigitalWrite( PA2, FUN_LOW );
		funDigitalWrite( PC4, FUN_LOW );
		funDigitalWrite( PD2, FUN_LOW );
		funDigitalWrite( PC1, FUN_LOW ); // 5v tolerant, no ADC

		Delay_Ms(1000);

		printf( "0: %d %d %d %d\n",
			funAnalogRead( ANALOG_0 ),
			funAnalogRead( ANALOG_1 ),
			funAnalogRead( ANALOG_2 ),
			funAnalogRead( ANALOG_3 ) );

		printf( "   %d %d %d %d %d\n\n",
			funDigitalRead( PA1 ),
			funDigitalRead( PA2 ),
			funDigitalRead( PC4 ),
			funDigitalRead( PD2 ),
			funDigitalRead( PC1 ) );
	}
}

