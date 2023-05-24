#ifndef WIRING_H
#define WIRING_H

#include "ch32v003fun.h"



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
	pin_A1 = 1,
	pin_A2,
	pin_C0 = 16,
	pin_C1,
	pin_C2,
	pin_C3,
	pin_C4,
	pin_C5,
	pin_C6,
	pin_C7,
	pin_D0 = 24,
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

enum ANALOGinputs {
	Ain0_A2,
	Ain1_A1,
	Ain2_C4,
	Ain3_D2,
	Ain4_D3,
	Ain5_D5,
	Ain6_D6,
	Ain7_D4,
	AinVref,
	AinVcal,
};

enum ANALOGsampletimes {
	Ast_3cy,
	Ast_9cy,
	Ast_15cy,
	Ast_30cy,
	Ast_43cy,
	Ast_57cy,
	Ast_73cy,
	Ast_241cy_default,
};


enum GPIOports getPort (enum GPIOpins pin);

void portEnable(enum GPIOports port);
void pinMode(enum GPIOpins pin, enum GPIOpinMode mode);
void digitalWrite(enum GPIOpins pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);

void ADCinit();
void ADCsetPower(uint8_t enable);
void ADCsetSampletime(enum ANALOGinputs input, enum ANALOGsampletimes time);
void ADCcalibrate();
uint16_t analogRead(enum ANALOGinputs input);

#endif					// WIRING_H
