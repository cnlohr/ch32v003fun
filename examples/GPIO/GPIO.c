// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>
#include "wiring_digital.h"

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main()
{
	SystemInit48HSI();

	// Enable GPIO ports
	enablePort(port_C);
	enablePort(port_D);

	// GPIO D0 Push-Pull
	pinMode(pin_D0, pinMode_O_pushPull);

	// GPIO D4 Push-Pull
	pinMode(pin_D4, pinMode_O_pushPull);

	// GPIO C0 Push-Pull
	pinMode(pin_C0, pinMode_O_pushPull);

	while(1)
	{
		// Turn on pins
		digitalWrite(pin_C0, high);
		digitalWrite(pin_D0, high);
		digitalWrite(pin_D4, high);
		Delay_Ms( 250 );
		// Turn off pins
		digitalWrite(pin_C0, low);
		digitalWrite(pin_D0, low);
		digitalWrite(pin_D4, low);
		Delay_Ms( 250 );
		for (int i = pin_A1; i < pin_D7; i++) {
			digitalWrite(i, high);
			Delay_Ms(50);
		}
		for (int i = pin_D7; i > pin_A1; i--) {
			digitalWrite(i, low);
			Delay_Ms(50);
		}
		count++;
	}
}

