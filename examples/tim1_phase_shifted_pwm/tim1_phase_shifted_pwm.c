#include "ch32fun.h"
#include <stdbool.h>
#include <stdio.h>

#define PWM1_PIN PC3
#define PWM2_PIN PC6

// system clock is 48MHz, so minimum frequency is 48MHz/65529 = 732Hz
#define TIM1_PRESCALER 1
#define OUTPUT_FREQUENCY 10000L

// correction factor to compensate for DMA and shadow register delays of 1/8µs
#define DUTY_CORRECTION 6

static uint16_t pwm_duty = 0;

/**
 * @brief Enables the PWM output on TIM1.
 *
 * This function enables the PWM output on TIM1 by setting the TIM_CEN bit in the
 * TIM1->CTLR1 register. It also resets the TIM1 counter to 0.
 */
static inline void enable_pwm( void )
{
	TIM1->CNT = 0;
	TIM1->CTLR1 |= TIM_CEN;
}

static inline void disable_pwm( void )
{
	TIM1->CTLR1 &= ~TIM_CEN;
}

/**
 * @brief Initializes the GPIO pins for the PWM1 and PWM2 signals.
 *
 * This function sets up the GPIO pins for the PWM1 (PC3) and PWM2 (PC6) signals.
 * It enables the clocks for TIM1 and the AFIO peripheral, and configures the GPIO
 * pins for alternate function push-pull output at 50 MHz.
 */
static void io_init( void )
{
	funGpioInitAll();

	// Enable TIM1 and AFIO clocks
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO;

	funPinMode( PWM1_PIN, GPIO_CFGLR_OUT_50Mhz_AF_PP );
	funPinMode( PWM2_PIN, GPIO_CFGLR_OUT_50Mhz_PP );
}

/**
 * @brief Sets the PWM period for TIM1.
 *
 * This function disables the PWM, updates the PWM period register (TIM1->ATRLR),
 * and then re-enables the PWM. This allows the PWM frequency to be updated
 * dynamically.
 *
 * @param period The new PWM period value.
 */
static void set_pwm_period( uint16_t period )
{
	disable_pwm();
	TIM1->ATRLR = period;
	enable_pwm();
}

static inline uint16_t get_pwm_period( void )
{
	return TIM1->ATRLR;
}

/**
 * @brief Sets the PWM frequency for TIM1.
 *
 * This function calculates the new PWM period value based on the provided frequency
 * and the system core clock, and then calls `set_pwm_period()` to update the PWM
 * period register and re-enable the PWM output.
 *
 * @param frequency The new PWM frequency value, in Hz.
 */
static void set_pwm_frequency( uint32_t frequency )
{
	set_pwm_period( FUNCONF_SYSTEM_CORE_CLOCK / frequency );
}

/**
 * @brief Sets the duty cycle of the PWM output on TIM1 channel 3 (PC3).
 *
 * This function disables the PWM, updates the duty cycle registers, and then re-enables the PWM.
 * It also updates the duty cycle for the complementary PWM output on PC6, which is used for
 * synchronization purposes.
 *
 * @param duty The new duty cycle value, between 0 (0% duty cycle) and 0xFFFF (100% duty cycle).
 */

static void set_pwm_duty( uint16_t duty )
{
	disable_pwm();
	pwm_duty = duty;

	// if phase shift is below ~140ns just leave the PWM disabled and return.
	uint16_t half_duty = pwm_duty / 2;
	if ( half_duty < DUTY_CORRECTION )
	{
		return;
	}
	half_duty -= DUTY_CORRECTION;

	uint16_t half_period = get_pwm_period() / 2;

	TIM1->CH3CVR = half_period; // channel 3, controls PC3 duty cycle

	TIM1->CH1CVR = half_duty; // channel 1, triggers rising edge of PC6
	TIM1->CH2CVR = half_period + half_duty; // channel 2, triggers falling edge of PC6

	enable_pwm();
}

/**
 * @brief Sets the duty cycle of the PWM output as a percentage.
 *
 * This function rounds the percentage value to the nearest duty cycle value and calls
 * `set_pwm_duty()` to update the PWM output.
 *
 * @param percentx100 The new duty cycle percentage, multiplied by 100 (e.g. 50% is 5000).
 */
static void set_pwm_duty_percent_x100( uint16_t percentx100 )
{
	// round to nearest duty cycle
	set_pwm_duty( (uint16_t)( ( ( ( (uint32_t)get_pwm_period() * percentx100 ) + 5000L ) / 10000L ) ) );
}

/**
 * @brief Initializes the PWM output on TIM1 channel 3 (PC3).
 *
 * This function sets up the TIM1 timer to generate a single-ended PWM signal on PC3.
 * It configures the timer prescaler, auto-reload value, and output compare modes.
 * The function also disables the complementary PWM output on PC6 and enables the
 * TIM1 outputs, then enables DMA events on TIM1 channels 1 and 2.
 */
static void t1pwm_init( void )
{
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_FULLREMAP;

	// single-ended PWM on PC3 (TIM1_CH3), active high
	TIM1->CCER |= TIM_CC3E;
	TIM1->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1; // mode=110 PC3 goes low on compare
	funDigitalWrite( PWM2_PIN, FUN_LOW ); // hold PC6 low

	TIM1->CTLR1 = 0; // CTLR1: default is up, events generated, edge align
	TIM1->CTLR2 = 0; // CTLR2: set output idle states (MOE off) via OIS1 and OIS1N bits

	// SMCFGR: default clk input is 48MHz CK_INT
	TIM1->PSC = TIM1_PRESCALER - 1; // divide by 1
	TIM1->ATRLR = 0x8000;

	set_pwm_duty( 0 );

	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;

	// Enable TIM1 outputs (also see OSSI and OSSR bits)
	TIM1->BDTR |= TIM_MOE;

	TIM1->DMAINTENR |= TIM_CC1DE | TIM_CC2DE; // DMA trigger on ch1 and ch2 compare matches
}

static inline void disable_dma( void )
{
	DMA1_Channel2->CFGR &= ~DMA_CFGR2_EN;
	DMA1_Channel3->CFGR &= ~DMA_CFGR2_EN;
}

static inline void enable_dma( void )
{
	DMA1_Channel2->CFGR |= DMA_CFGR2_EN;
	DMA1_Channel3->CFGR |= DMA_CFGR2_EN;
}

/**
 * @brief Initializes the DMA channels for TIM1 channel 1 and channel 2.
 *
 * This function sets up the DMA1 channels 2 and 3 to handle the DMA transfers
 * for TIM1 channel 1 and channel 2 respectively. It configures the memory and
 * peripheral addresses, transfer count, and DMA options such as priority,
 * data size, and transfer direction. The function also enables the DMA channels
 * after the configuration is complete.
 *
 * The DMA channels are used to directly set and reset the GPIO pin for the PWM2
 * pin (PC6) using the GPIO_BSHR register.
 */
static void t1dma_init( void )
{
	static uint32_t pc6_high = GPIO_BSHR_BS6;
	static uint32_t pc6_low = GPIO_BSHR_BR6;

	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1; // Turn on DMA1 clock

	disable_dma();

	// DMA1_Channel2 is for TIM1_CH1, memory -> peripheral
	DMA1_Channel2->MADDR = (uint32_t)&pc6_high;
	DMA1_Channel2->PADDR = (uint32_t)&GPIOC->BSHR;
	DMA1_Channel2->CNTR = 1;
	DMA1_Channel2->CFGR = DMA_M2M_Disable | DMA_Priority_VeryHigh | DMA_MemoryDataSize_Word |
	                      DMA_PeripheralDataSize_Word | DMA_MemoryInc_Disable | DMA_Mode_Circular |
	                      DMA_DIR_PeripheralDST | DMA_PeripheralInc_Disable;

	// DMA1_Channel3 is for TIM1_CH2, memory -> peripheral
	DMA1_Channel3->MADDR = (uint32_t)&pc6_low;
	DMA1_Channel3->PADDR = (uint32_t)&GPIOC->BSHR;
	DMA1_Channel3->CNTR = 1;
	DMA1_Channel3->CFGR = DMA_M2M_Disable | DMA_Priority_VeryHigh | DMA_MemoryDataSize_Word |
	                      DMA_PeripheralDataSize_Word | DMA_MemoryInc_Disable | DMA_Mode_Circular |
	                      DMA_DIR_PeripheralDST | DMA_PeripheralInc_Disable;

	enable_dma();
}


/**
 * @brief Main entry point of the program.
 *
 * This function initializes the system, sets up the I/O, DMA, and PWM, and then
 * enters an infinite loop that sweeps the PWM duty cycle every 2 seconds from 9% to 99% in 10%
 * increments, printing the current duty cycle and channel values to the debugger (if attached).
 * The program uses DMA to directly update the GPIO pin for the PWM output,
 * allowing for precise timing and low CPU utilization.
 */
int main( void )
{
	SystemInit();

	io_init();
	t1dma_init();
	t1pwm_init();
	set_pwm_frequency( OUTPUT_FREQUENCY);

	bool debugger_connected = DidDebuggerAttach();

	for ( ;; )
	{
		for ( uint16_t percentx100 = 900; percentx100 < 10000; percentx100 += 1000 )
		{
			set_pwm_duty_percent_x100( percentx100 );
			if ( debugger_connected )
			{
				printf( "pct: %d, DC: %d, CH1: %ld, CH2: %ld, CH3: %ld\n", percentx100, pwm_duty, TIM1->CH1CVR,
					TIM1->CH2CVR, TIM1->CH3CVR );
			}
			Delay_Ms( 2000 );
		}
	}
}
