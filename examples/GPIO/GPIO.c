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

	while (1) {
		// Turn on pins
		GPIO_digitalWrite(GPIO_pin_C0, high);
		GPIO_digitalWrite(GPIO_pin_D4, high);
		Delay_Ms(250);
		// Turn off pins
		GPIO_digitalWrite(GPIO_pin_C0, low);
		GPIO_digitalWrite(GPIO_pin_D4, low);
		Delay_Ms(250);
		for (int i = GPIO_pin_C0; i <= GPIO_pin_C7; i++) {
			GPIO_digitalWrite(i, high);
			Delay_Ms(50);
		}
		for (int i = GPIO_pin_C7; i >= GPIO_pin_C0; i--) {
			GPIO_digitalWrite(i, low);
			Delay_Ms(50);
		}
		Delay_Ms(250);
		count++;
	}
}
