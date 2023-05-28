//#include <stdio.h>

#include "wiring.h"
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



void portEnable(enum GPIOports port) {
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
	uint16_t PinOffset;

	GPIOx = (GPIO_TypeDef *)(((uint8_t*)(GPIOA))+(pin>>3)*0x0400);
	PinOffset = (pin & 0x7)<<2;

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
		PinOffset = (pin - 16);
	}
	else if (pin <= pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 24);
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
		PinOffset = (pin - 16);
	}
	else if (pin <= pin_D7) {
		GPIOx = GPIOD;
		PinOffset = (pin - 24);
	}
	else {
		return 0;
	}

	int8_t result = (GPIOx->INDR >> PinOffset) & 1;
	return result;
}





void ADCinit() {
	// select ADC clock source
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	RCC->CFGR0 &= ~(0x1F<<11);

	// enable clock to the ADC
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// set sampling time for all inputs to 241 cycles
	for (uint8_t i = Ain0_A2; i <= AinVcal; i++) {
		ADCsetSampletime(i, Ast_241cy_default);
	}

	// set trigger to software
	ADC1->CTLR2 |= ADC_EXTSEL;

	// pre-clear conversion queue
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0;

	// power the ADC
	ADCsetPower(1);
}



void ADCsetSampletime(enum ANALOGinputs input, enum ANALOGsampletimes time) {
	// clear
	ADC1->SAMPTR2 &= ~(0b111)<<(3*input);
	// set
	ADC1->SAMPTR2 |= time<<(3*input);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
}



void ADCsetPower(uint8_t enable) {
	if (enable) {
		ADC1->CTLR2 |= ADC_ADON;
		if (enable == 1) {
			// auto-cal each time after turning on the ADC
			// can be overridden by calling with enable > 1.
			ADCcalibrate();					
		}
	}
	else {
		ADC1->CTLR2 &= ~(ADC_ADON);
	}
}



void ADCcalibrate() {
	// reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
}



// inspired by arduinos analogRead()
uint16_t analogRead(enum ANALOGinputs input) {
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
