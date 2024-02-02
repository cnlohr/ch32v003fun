/*
 * Example of a useless code. It doesn't do anything good except being great at what it does.
 * It demonstrates the power of repetition: if you repeat the samething many times over, it becomes you.
 * 
 * fun_gpio.h is based on ch32v_hal.h by Larry Bank
 * fun_timer.h is based on Example SysTick with IRQs work of E. Brombaugh and C. Lohr
 * by unicab369
 */

#include "ch32v003fun.h"
#include <stdio.h>
#include "fun_button.h"

#define BUTTON_PIN 0xC0
#define LED_PIN 0xD0

void button_onSingleClick() {
   printf("\nI'M USELESS.");
	digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void button_onDoubleClick() {
   printf("\nI'M USELESS TWICE.");
}

int main()
{  
	pinMode(LED_PIN, OUTPUT);
	button_setup(BUTTON_PIN);

	printf("looping...\n\r");
	while(1)
	{
		button_run();
	}
}

