//######## necessities

// include guards
#ifndef CH32V003_GPIO_BR_H
#define CH32V003_GPIO_BR_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description
This is a speedy and light GPIO library due to
	static inlining of most functions
	compile-time abstraction
	branchless where it counts
*/



/*######## library usage and configuration
 * in your .c file, unlock this libraries implementation block:
then
#include "../../extralibs/ch32v003_GPIO_branching.h"

first, enable the desired port.

digital usage is quite Arduino-like:
pinMode
digitalWrite
digitalWrite_lo
digitalWrite_hi
digitalRead

analog usage is almost Arduino-like:
ADCinit
pinMode
analogRead

you can also disable the ADC to save power

there is no analogWrite (yet)


to unlock the ability to easily iterate over the pins
#define CH32V003_GPIO_ITER_IMPLEMENTATION
before the #include
this unlocks the GPIO_from_pinNumber function you can use to derive the desired pins from a simple pinNumber
0 .. 1		A1 .. A2
2 .. 9		C0 .. C7
10..17		D0 .. D7

*/



//######## pins and states: use these for the functions below!

// GPIO ports (for enabling)
#define GPIO_port_EN_A RCC_APB2Periph_GPIOA
#define GPIO_port_EN_C RCC_APB2Periph_GPIOC
#define GPIO_port_EN_D RCC_APB2Periph_GPIOD

// GPIO pin modes
#define GPIO_pinMode_I_floating		GPIO_CNF_IN_FLOATING
#define GPIO_pinMode_I_pullUp		GPIO_CNF_IN_PUPD	| 0b00110000		//pull-mode + BSHR(1)
#define GPIO_pinMode_I_pullDown		GPIO_CNF_IN_PUPD	| 0b00010000		//pull-mode + BSHR(0)
#define GPIO_pinMode_I_analog		GPIO_CNF_IN_ANALOG
#define GPIO_pinMode_O_pushPull		GPIO_CNF_OUT_PP		| GPIO_Speed_10MHz
#define GPIO_pinMode_O_openDrain	GPIO_CNF_OUT_OD		| GPIO_Speed_10MHz
#define GPIO_pinMode_O_pushPullMux	GPIO_CNF_OUT_PP_AF	| GPIO_Speed_10MHz
#define GPIO_pinMode_O_openDrainMux	GPIO_CNF_OUT_OD_AF	| GPIO_Speed_10MHz
// bit layout 00TPMMMM
// P enables pull resistors, T sets pull resistors high / low

// GPIO pins, use for all functions!
#define GPIO_pin_A1	(GPIOA_BASE + 1)
#define GPIO_pin_A2	(GPIOA_BASE + 2)
#define GPIO_pin_C0	(GPIOC_BASE + 0)
#define GPIO_pin_C1	(GPIOC_BASE + 1)
#define GPIO_pin_C2	(GPIOC_BASE + 2)
#define GPIO_pin_C3	(GPIOC_BASE + 3)
#define GPIO_pin_C4	(GPIOC_BASE + 4)
#define GPIO_pin_C5	(GPIOC_BASE + 5)
#define GPIO_pin_C6	(GPIOC_BASE + 6)
#define GPIO_pin_C7	(GPIOC_BASE + 7)
#define GPIO_pin_D0	(GPIOD_BASE + 0)
#define GPIO_pin_D1	(GPIOD_BASE + 1)
#define GPIO_pin_D2	(GPIOD_BASE + 2)
#define GPIO_pin_D3	(GPIOD_BASE + 3)
#define GPIO_pin_D4	(GPIOD_BASE + 4)
#define GPIO_pin_D5	(GPIOD_BASE + 5)
#define GPIO_pin_D6	(GPIOD_BASE + 6)
#define GPIO_pin_D7	(GPIOD_BASE + 7)

// GPIO pins, for iterator boundaries only!
enum GPIO_pinNumbers{
	GPIO_pinNumber_A1,
	GPIO_pinNumber_A2,
	GPIO_pinNumber_C0,
	GPIO_pinNumber_C1,
	GPIO_pinNumber_C2,
	GPIO_pinNumber_C3,
	GPIO_pinNumber_C4,
	GPIO_pinNumber_C5,
	GPIO_pinNumber_C6,
	GPIO_pinNumber_C7,
	GPIO_pinNumber_D0,
	GPIO_pinNumber_D1,
	GPIO_pinNumber_D2,
	GPIO_pinNumber_D3,
	GPIO_pinNumber_D4,
	GPIO_pinNumber_D5,
	GPIO_pinNumber_D6,
	GPIO_pinNumber_D7,
};

enum lowhigh {
	low,
	high,
};

// analog inputs
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

// how many cycles the ADC shall sample the input for (speed vs precision)
enum GPIO_ADC_sampletimes {
	GPIO_ADC_sampletime_3cy,
	GPIO_ADC_sampletime_9cy,
	GPIO_ADC_sampletime_15cy,
	GPIO_ADC_sampletime_30cy,
	GPIO_ADC_sampletime_43cy,
	GPIO_ADC_sampletime_57cy,
	GPIO_ADC_sampletime_73cy,
	GPIO_ADC_sampletime_241cy_default,
};

enum GPIO_ports{
	GPIO_port_A,
	GPIO_port_C,
	GPIO_port_D,
	GPIO_port_none,
};



//######## function overview (declarations): use these!

static inline void GPIO_portEnable(uint32_t port_EN) __attribute__((used));

static inline void GPIO_pinMode (uint32_t pin, uint8_t pinMode);

static inline void GPIO_digitalWrite_hi(uint32_t pin);				// completely branchless -> maximum speed
static inline void GPIO_digitalWrite_lo(uint32_t pin);				// completely branchless -> maximum speed
static inline void GPIO_digitalWrite(uint32_t pin, enum lowhigh direction);

static inline uint8_t GPIO_digitalRead(uint32_t pin);				// completely branchless -> maximum speed


static inline void GPIO_ADCinit();
static inline void GPIO_ADC_set_sampletime(enum GPIO_analog_inputs input, enum GPIO_ADC_sampletimes time);
static inline void GPIO_ADC_set_power(uint8_t enable);
static inline void GPIO_ADC_calibrate();

static inline uint16_t GPIO_analogRead(enum GPIO_analog_inputs input);

// helper for easy addressing of pins with an iterator (for-loop, while-loop)
// enable with #define CH32V003_GPIO_ITER_IMPLEMENTATION before the #include
// only function that expects a pinNumber (see enum above)
static inline uint32_t GPIO_from_pinNumber(uint8_t pinNumber);



//######## internal function declarations

enum GPIO_ports GPIO_getPort (uint32_t pin);


//######## internal variables



//######## preprocessor macros

#define GPIO_get_pin_PORTX_REG(pin) ((GPIO_TypeDef*)(uintptr_t)(pin & (~0b1111)))
#define GPIO_get_pin_N(pin) (pin & 0b1111)
#define GPIO_get_pull_en(pinMode) (pinMode >> 4) & 0b1
#define GPIO_get_pull_direction(pinMode) (pinMode >> 5) & 0b1



//######## preprocessor #define requirements



//######## small function definitions, static inline

// __attribute__((used)) is needed in the declaration so compiler doesn't optimize this into oblivion
static inline void GPIO_portEnable(uint32_t port_EN) {
	RCC->APB2PCENR |= port_EN;
}

static inline void GPIO_pinMode (uint32_t pin, uint8_t pinMode) {
	// zero the 4 configuration bits
	GPIO_get_pin_PORTX_REG(pin)->CFGLR &= ~(0b1111 << (4 * GPIO_get_pin_N(pin)));
	// write the correct 4 configuration bits
	GPIO_get_pin_PORTX_REG(pin)->CFGLR |= (0b1111 & pinMode) << (4 * GPIO_get_pin_N(pin));
	// set output state if pushhigh if pullup, else down
	if (GPIO_get_pull_en(pinMode)) {
		GPIO_get_pin_PORTX_REG(pin)->BSHR = GPIO_get_pull_direction(pinMode) << GPIO_get_pin_N(pin);
	}
}

static inline void GPIO_digitalWrite_hi(uint32_t pin) {
	GPIO_get_pin_PORTX_REG(pin)->BSHR = 1 << GPIO_get_pin_N(pin);
}
static inline void GPIO_digitalWrite_lo(uint32_t pin) {
	GPIO_get_pin_PORTX_REG(pin)->BSHR = 1 << (GPIO_get_pin_N(pin) + 16);
} 
static inline void GPIO_digitalWrite(uint32_t pin, enum lowhigh direction) {
	// no checks given whether pin is currently being toggled by timer! your output trannys are in your hands! beware the magic smoke!
	if (direction) {
		GPIO_digitalWrite_hi(pin);
	}
	else {
		GPIO_digitalWrite_lo(pin);
	}
}

static inline uint8_t GPIO_digitalRead(uint32_t pin) {
	return GPIO_get_pin_PORTX_REG(pin)->INDR >> GPIO_get_pin_N(pin) & 0b1;
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
		GPIO_ADC_set_sampletime(i, GPIO_ADC_sampletime_241cy_default);
	}

	// set trigger to software
	ADC1->CTLR2 |= ADC_EXTSEL;

	// pre-clear conversion queue
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0;

	// power the ADC
	GPIO_ADC_set_power(1);
}

static inline void GPIO_ADC_set_sampletime(enum GPIO_analog_inputs input, enum GPIO_ADC_sampletimes time) {
	// clear
	ADC1->SAMPTR2 &= ~(0b111)<<(3*input);
	// set
	ADC1->SAMPTR2 |= time<<(3*input);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
}

static inline void GPIO_ADC_set_power(uint8_t enable) {
	if (enable) {
		ADC1->CTLR2 |= ADC_ADON;
		if (enable == 1) {
			// auto-cal each time after turning on the ADC
			// can be skipped by calling with enable > 1.
			GPIO_ADC_calibrate();					
		}
	}
	else {
		ADC1->CTLR2 &= ~(ADC_ADON);
	}
}

static inline void GPIO_ADC_calibrate() {
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
	// other micro controllers insert a full ms delay right here!
	
	// start sw conversion (auto clears)
	ADC1->CTLR2 |= ADC_SWSTART;
	
	// wait for conversion complete
	while(!(ADC1->STATR & ADC_EOC)) {}
	
	// get result
	return ADC1->RDATAR;
}

//######## small internal function definitions, static inline



//######## implementation block
#define CH32V003_GPIO_ITER_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_GPIO_ITER_IMPLEMENTATION)

// index to pin
uint32_t GPIO_pinNumbers_iterable[18] = {
	GPIO_pin_A1, GPIO_pin_A2,
	GPIO_pin_C0, GPIO_pin_C1, GPIO_pin_C2, GPIO_pin_C3, GPIO_pin_C4, GPIO_pin_C5, GPIO_pin_C6, GPIO_pin_C7,
	GPIO_pin_D0, GPIO_pin_D1, GPIO_pin_D2, GPIO_pin_D3, GPIO_pin_D4, GPIO_pin_D5, GPIO_pin_D6, GPIO_pin_D7,
};

// helper for iterating over the pins
static inline uint32_t GPIO_from_pinNumber(uint8_t pinNumber) {
	return GPIO_pinNumbers_iterable[pinNumber];
}

// can't really think of a better way of finding the port number (0 for A, 1 for C, 2 for D);
// not strictly used but wouldn't be bad to have
enum GPIO_ports GPIO_getPort (uint32_t pin) {
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
// con't solve it with a #define (/1024 or /2048) either because of nonexistent GPIOB in between!
// please improve it if you know how!



#endif // CH32V003_GPIO_ITER_IMPLEMENTATION
#endif // CH32V003_GPIO_BR_H
