// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "../../ch32v003fun/ch32v003fun.h"

#define CH32V003_GPIO_BR_IMPLEMENTATION
#include "../../extralibs/ch32v003_GPIO_branching.h"

#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main() {
	SystemInit48HSI();

	// Enable GPIO ports
	GPIO_portEnable(GPIO_port_C);
	GPIO_portEnable(GPIO_port_D);

	for (int i = GPIO_pin_C0; i <= GPIO_pin_C7; i++) {
		GPIO_pinMode(i, GPIO_pinMode_O_pushPull);
	}

	// GPIO D4 Push-Pull
	GPIO_pinMode(GPIO_pin_D4, GPIO_pinMode_O_pushPull);

	GPIO_pinMode(GPIO_pin_D6, GPIO_pinMode_I_analog);
	GPIO_ADCinit();

	while (1) {
		GPIO_digitalWrite(GPIO_pin_D4, high);
		uint8_t leds_to_turn_on = (uint8_t)(((float)(GPIO_analogRead(GPIO_Ain6_D6)) / 1024.f) * 8.f * 1.2 - 1.f);
		uint8_t led_i = 0;
		for (int i = GPIO_pin_C0; i <= GPIO_pin_C7; i++) {
			if (led_i < leds_to_turn_on) {
				GPIO_digitalWrite(i, high);
			}
			else {
				GPIO_digitalWrite(i, low);
			}
			led_i++;
		}
		GPIO_digitalWrite(GPIO_pin_D4, low);
		Delay_Ms(250);
		count++;
	}
}
