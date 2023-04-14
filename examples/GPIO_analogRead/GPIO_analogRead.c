// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include "wiring.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main() {
	SystemInit48HSI();

	// Enable GPIO ports
	portEnable(port_C);
	portEnable(port_D);

	for (int i = pin_C0; i <= pin_C7; i++) {
		pinMode(i, pinMode_O_pushPull);
	}

	// GPIO D4 Push-Pull
	pinMode(pin_D4, pinMode_O_pushPull);

	pinMode(pin_D6, pinMode_I_analog);
	ADCinit();

	while (1) {
		digitalWrite(pin_D4, high);
		uint8_t leds_to_turn_on = (uint8_t)(((float)(analogRead(Ain6_D6)) / 1024.f) * 8.f * 1.2 - 1.f);
		uint8_t led_i = 0;
		for (int i = pin_C0; i <= pin_C7; i++) {
			if (led_i < leds_to_turn_on) {
				digitalWrite(i, high);
			}
			else {
				digitalWrite(i, low);
			}
			led_i++;
		}
		digitalWrite(pin_D4, low);
		Delay_Ms(250);
		count++;
	}
}
