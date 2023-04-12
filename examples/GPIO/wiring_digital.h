#ifndef WIRING_DIGITAL_H
#define WIRING_DIGITAL_H

#include "../../ch32v003fun/ch32v003fun.h"

// Define the pins that will be used for GPIO
#define MY_GPIO_PIN_1 1
#define MY_GPIO_PIN_2 2
// Add more pins as needed



enum lowhigh {
	low,
	high,
};



enum GPIOports{
	port_A,
	port_C,
	port_D,
	port_none,
};

enum GPIOpins{
	pin_A1,
	pin_A2,
	pin_C0,
	pin_C1,
	pin_C2,
	pin_C3,
	pin_C4,
	pin_C5,
	pin_C6,
	pin_C7,
	pin_D0,
	pin_D1,
	pin_D2,
	pin_D3,
	pin_D4,
	pin_D5,
	pin_D6,
	pin_D7,
	pin_none,
};

enum GPIOpinMode {
	pinMode_I_floating,
	pinMode_I_pullUp,			//pull-mode + ODR(1)
	pinMode_I_pullDown,			//pull-mode + ODR(0)
	pinMode_I_analog,
	pinMode_O_pushPull,
	pinMode_O_openDrain,
	pinMode_O_pushPullMux,
	pinMode_O_openDrainMux,
};

enum GPIOpinState {
	pinState_nochange,
	pinState_low,
	pinState_high,
};

enum GPIOports getPort (enum GPIOpins pin);

void enablePort(enum GPIOports port);
void pinMode(enum GPIOpins pin, enum GPIOpinMode mode);
void digitalWrite(enum GPIOpins pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);

#endif					// WIRING_DIGITAL_H
