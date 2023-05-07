//######## necessities

// include guards
#ifndef CH32V003_GPIO_BR_H
#define CH32V003_GPIO_BR_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description
This is the branching GPIO library.

advantages:
easier to use
ability to iterate over pins

disadvantages:
function calls for digital GPIO interactions!
	slower
run-time branching!
	slower
	larger binary size

This is the right library to choose if the actual pin to toggle and its state get evaluated at run-time / isn't known to you at compile-time.
*/



/*######## library usage and configuration
 * in your .c file, unlock this libraries implementation block:
#define CH32V003_GPIO_BR_IMPLEMENTATION
then
#include "../../extralibs/ch32v003_GPIO_branching.h"

first, enable the desired port.

digital usage is very Arduino-like:
pinMode
digitalWrite
digitalRead

analog usage is almost Arduino-like:
ADCinit
pinMode
analogRead

there is no analogWrite (yet)

you can also disable the ADC to save power
*/



//######## pins and states: use these!
enum lowhigh {
	low,
	high,
};

enum GPIO_ports{
	GPIO_port_A,
	GPIO_port_C,
	GPIO_port_D,
	GPIO_port_none,
};
enum GPIO_pins{
	GPIO_pin_A1,
	GPIO_pin_A2,
	GPIO_pin_C0,
	GPIO_pin_C1,
	GPIO_pin_C2,
	GPIO_pin_C3,
	GPIO_pin_C4,
	GPIO_pin_C5,
	GPIO_pin_C6,
	GPIO_pin_C7,
	GPIO_pin_D0,
	GPIO_pin_D1,
	GPIO_pin_D2,
	GPIO_pin_D3,
	GPIO_pin_D4,
	GPIO_pin_D5,
	GPIO_pin_D6,
	GPIO_pin_D7,
	GPIO_pin_none,
};

enum GPIO_pinModes {
	GPIO_pinMode_I_floating,
	GPIO_pinMode_I_pullUp,				//pull-mode + ODR(1)
	GPIO_pinMode_I_pullDown,			//pull-mode + ODR(0)
	GPIO_pinMode_I_analog,
	GPIO_pinMode_O_pushPull,
	GPIO_pinMode_O_openDrain,
	GPIO_pinMode_O_pushPullMux,
	GPIO_pinMode_O_openDrainMux,
};

enum GPIO_pinStates {
	GPIO_pinState_nochange,
	GPIO_pinState_low,
	GPIO_pinState_high,
};

enum GPIO_analog_inputs {
	GPIO_Ain0_A2,
	GPIO_Ain1_A1,
	GPIO_Ain2_C4,
	GPIO_Ain3_D2,
	GPIO_Ain4_D3,
	GPIO_Ain5_D5,
	GPIO_Ain6_D6,
	GPIO_Ain7_D4,
	GPIO_AinVref,
	GPIO_AinVcal,
};

enum GPIO_analog_sampletimes {
	GPIO_Ast_3cy,
	GPIO_Ast_9cy,
	GPIO_Ast_15cy,
	GPIO_Ast_30cy,
	GPIO_Ast_43cy,
	GPIO_Ast_57cy,
	GPIO_Ast_73cy,
	GPIO_Ast_241cy_default,
};



//######## function overview (declarations): use these!

static inline void GPIO_portEnable(enum GPIO_ports port);

void GPIO_pinMode(enum GPIO_pins pin, enum GPIO_pinModes mode);

static inline void GPIO_ADCinit();
static inline void GPIO_ADCsetSampletime(enum GPIO_analog_inputs input, enum GPIO_analog_sampletimes time);

static inline void GPIO_ADCsetPower(uint8_t enable);
static inline void GPIO_ADCcalibrate();

void GPIO_digitalWrite(enum GPIO_pins pin, uint8_t value);
uint8_t GPIO_digitalRead(uint8_t pin);
static inline uint16_t GPIO_analogRead(enum GPIO_analog_inputs input);

enum GPIO_ports GPIO_getPort (enum GPIO_pins pin);



//######## internal function declarations



//######## internal variables



//######## preprocessor macros



//######## preprocessor #define requirements
#if !(defined(CH32V003_GPIO_BR_IMPLEMENTATION))
      #error "please unlock the CH32V003_GPIO_BR_IMPLEMENTATION with a #define before the #include!"
#endif



//######## small function definitions, static inline

static inline void GPIO_portEnable(enum GPIO_ports port) {
	// Enable GPIOs
	switch (port) {
		case GPIO_port_A:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
			break;
		case GPIO_port_C:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
			break;
		case GPIO_port_D:
			RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
			break;
		case GPIO_port_none:
			break;
	}
}



// large but will likely only ever be called once
static inline void GPIO_ADCinit() {
	// select ADC clock source
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	RCC->CFGR0 &= ~(0x1F<<11);

	// enable clock to the ADC
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// set sampling time for all inputs to 241 cycles
	for (uint8_t i = GPIO_Ain0_A2; i <= GPIO_AinVcal; i++) {
		GPIO_ADCsetSampletime(i, GPIO_Ast_241cy_default);
	}

	// set trigger to software
	ADC1->CTLR2 |= ADC_EXTSEL;

	// pre-clear conversion queue
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0;

	// power the ADC
	GPIO_ADCsetPower(1);
}

static inline void GPIO_ADCsetSampletime(enum GPIO_analog_inputs input, enum GPIO_analog_sampletimes time) {
	// clear
	ADC1->SAMPTR2 &= ~(0b111)<<(3*input);
	// set
	ADC1->SAMPTR2 |= time<<(3*input);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
}
static inline void GPIO_ADCsetPower(uint8_t enable) {
	if (enable) {
		ADC1->CTLR2 |= ADC_ADON;
		if (enable == 1) {
			// auto-cal each time after turning on the ADC
			// can be overridden by calling with enable > 1.
			GPIO_ADCcalibrate();					
		}
	}
	else {
		ADC1->CTLR2 &= ~(ADC_ADON);
	}
}
static inline void GPIO_ADCcalibrate() {
	// reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
}

static inline uint16_t GPIO_analogRead(enum GPIO_analog_inputs input) {
	// set mux to selected input
	ADC1->RSQR3 = input;

	// may need a delay right here for the mux to actually finish switching??
	// Arduino inserts a full ms delay right here!
	
	// start sw conversion (auto clears)
	ADC1->CTLR2 |= ADC_SWSTART;
	
	// wait for conversion complete
	while(!(ADC1->STATR & ADC_EOC));
	
	// get result
	return ADC1->RDATAR;
}

//######## small internal function definitions, static inline



//######## implementation block
#define CH32V003_GPIO_BR_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_GPIO_BR_IMPLEMENTATION)


enum GPIO_ports GPIO_getPort (enum GPIO_pins pin) {
	if (pin <= GPIO_pin_A2) {
		return GPIO_port_A;
	}
	else if (pin <= GPIO_pin_C7) {
		return GPIO_port_C;
	}
	else if (pin <= GPIO_pin_D7) {
		return GPIO_port_D;
	}
	return GPIO_port_none;
}

void GPIO_pinMode(enum GPIO_pins pin, enum GPIO_pinModes mode) {
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 4;
	
	if (pin <= GPIO_pin_A2) {
		GPIOx = GPIOA;
		PinOffset *= pin;
	}
	else if (pin <= GPIO_pin_C7) {
		GPIOx = GPIOC;
		PinOffset *= (pin - 2);
	}
	else if (pin <= GPIO_pin_D7) {
		GPIOx = GPIOD;
		PinOffset *= (pin - 10);
	}
	else {
		return;
	}

	GPIOx->CFGLR &= ~(0b1111<<PinOffset);							// zero the 4 configuration bits
	
	uint8_t target_pin_state = GPIO_pinState_nochange;					// by default, pin shall retain its state

	uint8_t modeMask = 0;												// configuration mask

	switch (mode) {
		case GPIO_pinMode_I_floating:
			modeMask = GPIO_CNF_IN_FLOATING;
			break;
		case GPIO_pinMode_I_pullUp:
			modeMask = GPIO_CNF_IN_PUPD;
			target_pin_state = GPIO_pinState_high;
			break;
		case GPIO_pinMode_I_pullDown:
			modeMask = GPIO_CNF_IN_PUPD;
			target_pin_state = GPIO_pinState_low;
			break;
		case GPIO_pinMode_I_analog:
			modeMask = GPIO_CNF_IN_ANALOG;
			break;
		case GPIO_pinMode_O_pushPull:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_PP;
			break;
		case GPIO_pinMode_O_openDrain:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_OD;
			break;
		case GPIO_pinMode_O_pushPullMux:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF;
			break;
		case GPIO_pinMode_O_openDrainMux:
			modeMask = GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF;
			break;
	}

	// wirte mask to CFGR
	GPIOx->CFGLR |= modeMask<<PinOffset;

	// set pin state
	if (target_pin_state > GPIO_pinState_nochange) {
		GPIO_digitalWrite(pin, target_pin_state - 1);
	}
}

void GPIO_digitalWrite(enum GPIO_pins pin, uint8_t value) {
	// no checks given whether pin is currently being toggled by timer! your output trannys are in your hands! beware the magic smoke!
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 0;
	
	if (pin <= GPIO_pin_A2) {
		GPIOx = GPIOA;
		PinOffset = pin;
	}
	else if (pin <= GPIO_pin_C7) {
		GPIOx = GPIOC;
		PinOffset = (pin - 2);
	}
	else if (pin <= GPIO_pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 10);
	}
	else {
		return;
	}

	if (value) {
		GPIOx-> BSHR = 1 << PinOffset;
	}
	else {
		GPIOx-> BSHR = 1 << (16 + PinOffset);
	}
}

uint8_t GPIO_digitalRead(uint8_t pin) {
	GPIO_TypeDef * GPIOx;
	uint16_t PinOffset = 0;
	
	if (pin <= GPIO_pin_A2) {
		GPIOx = GPIOA;
		PinOffset = pin;
	}
	else if (pin <= GPIO_pin_C7) {
		GPIOx = GPIOC;
		PinOffset = (pin - 2);
	}
	else if (pin <= GPIO_pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 10);
	}
	else {
		return 0;
	}

	int8_t result = (GPIOx->INDR >> PinOffset) & 1;
	return result;
}

#endif // CH32V003_GPIO_BR_IMPLEMENTATION
#endif // CH32V003_GPIO_BR_H
