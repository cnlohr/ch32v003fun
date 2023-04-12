//#include <stdio.h>

#include "wiring_digital.h"
#include <stdint.h>



enum GPIOports getPort (enum GPIOpins pin) {
	if (pin <= pin_A2) {
		return port_A;
	}
	else if (pin <= pin_C7) {
		return port_C;
	}
	else if (pin <= pin_D7) {
		return port_D;
	}
	return port_none;
}



void enablePort(enum GPIOports port) {
	// Enable GPIOs
	switch (port) {
		case port_A:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
			break;
		case port_C:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
			break;
		case port_D:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
			break;
		case port_none:
			break;
	}
}



void pinMode(enum GPIOpins pin, enum GPIOpinMode mode) {
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 4;
	
	if (pin <= pin_A2) {
		GPIOx = GPIOA;
		PinOffset *= pin;
	}
	else if (pin <= pin_C7) {
		GPIOx = GPIOC;
		PinOffset *= (pin - 2);
	}
	else if (pin <= pin_D7) {
		GPIOx = GPIOD;
		PinOffset *= (pin - 10);
	}
	else {
		return;
	}

	GPIOx->CFGLR &= ~(0b1111<<PinOffset);							// zero the 4 configuration bits
	
	uint8_t target_pin_state = pinState_nochange;					// by default, pin shall retain its state

	uint8_t modeMask = 0;												// configuration mask

	switch (mode) {
		case pinMode_I_floating:
			modeMask = GPIO_CNF_IN_FLOATING;
			break;
		case pinMode_I_pullUp:
			modeMask = GPIO_CNF_IN_PUPD;
			target_pin_state = pinState_high;
			break;
		case pinMode_I_pullDown:
			modeMask = GPIO_CNF_IN_PUPD;
			target_pin_state = pinState_low;
			break;
		case pinMode_I_analog:
			modeMask = GPIO_CNF_IN_ANALOG;
			break;
		case pinMode_O_pushPull:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_PP;
			break;
		case pinMode_O_openDrain:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_OD;
			break;
		case pinMode_O_pushPullMux:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF;
			break;
		case pinMode_O_openDrainMux:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF;
			break;
	}

	// wirte mask to CFGR
	GPIOx->CFGLR |= modeMask<<PinOffset;

	// set pin state
	if (target_pin_state > pinState_nochange) {
		digitalWrite(pin, target_pin_state - 1);
	}
}



void digitalWrite(enum GPIOpins pin, uint8_t value) {
	// no checks given whether pin is currently being toggled by timer! your output trannys are in your hands! beware the magic smoke!
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 0;
	
	if (pin <= pin_A2) {
		GPIOx = GPIOA;
		PinOffset = pin;
	}
	else if (pin <= pin_C7) {
		GPIOx = GPIOC;
		PinOffset = (pin - 2);
	}
	else if (pin <= pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 10);
	}
	else {
		return;
	}

	if (value) {
		GPIOx-> BSHR |= 1 << PinOffset;
	}
	else {
		GPIOx-> BSHR |= 1 << (16 + PinOffset);
	}
}



uint8_t digitalRead(uint8_t pin) {
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 0;
	
	if (pin <= pin_A2) {
		GPIOx = GPIOA;
		PinOffset = pin;
	}
	else if (pin <= pin_C7) {
		GPIOx = GPIOC;
		PinOffset = (pin - 2);
	}
	else if (pin <= pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 10);
	}
	else {
		return 0;
	}

	int8_t result = (GPIOx->INDR >> PinOffset) & 1;
	return result;
}
