// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "../../ch32v003fun/ch32v003fun.h"

#define CH32V003_GPIO_ITER_IMPLEMENTATION
#include "../../extralibs/ch32v003_GPIO_branchless.h"

#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main() {
	SystemInit48HSI();

	// Enable GPIO ports
	GPIO_portEnable(GPIO_port_EN_C);
	GPIO_portEnable(GPIO_port_EN_D);

	// GPIO C0 - C7 Push-Pull
	for (int i = GPIO_pinNumber_C0; i<= GPIO_pinNumber_C7; i++) {
		GPIO_pinMode(GPIO_from_pinNumber(i), GPIO_pinMode_O_pushPull);
	}

	// GPIO D4 Push-Pull
	GPIO_pinMode(GPIO_pin_D4, GPIO_pinMode_O_pushPull);

	GPIO_pinMode(GPIO_pin_D6, GPIO_pinMode_I_analog);
	GPIO_ADCinit();

	while (1) {
		GPIO_digitalWrite(GPIO_pin_D4, low);
		uint8_t leds_to_turn_on = (uint8_t)(((float)(GPIO_analogRead(GPIO_Ain6_D6)) / 1024.f) * 8.f * 1.2 - 1.f);
		uint8_t led_i = 0;
		for (int i = GPIO_pinNumber_C0; i<= GPIO_pinNumber_C7; i++) {
			if (led_i < leds_to_turn_on) {
				GPIO_digitalWrite_hi(GPIO_from_pinNumber(i));
			}
			else {
				GPIO_digitalWrite_lo(GPIO_from_pinNumber(i));
			}
			led_i++;
		}
		GPIO_digitalWrite(GPIO_pin_D4, high);
		Delay_Ms(50);
		count++;
	}
}
