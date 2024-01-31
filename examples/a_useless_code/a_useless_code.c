/*
 * Example of a useless code. It doesn't do anything good except being great at what it does.
 * It demonstrates the power of repetition: if you repeat the samething many times over, it becomes you.
 * by unicab369
 */

#include "ch32v003fun.h"
#include <stdio.h>
#include "fun_button.h"

void button_onSingleClick() {
   printf("\nI'M USELESS.");
	digitalWrite(0xC0, 0);
}

void button_onDoubleClick() {
   printf("\nI'M USELESS TWICE.");
	digitalWrite(0xC0, !digitalRead(0xC0));
}

int main()
{  
	pinMode(0xC0, OUTPUT);
	digitalWrite(0xC0, 1);

	button_setup(0xC3);

	printf("looping...\n\r");
	while(1)
	{
		button_run();
	}
}

