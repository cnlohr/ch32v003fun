/*
 * Single-File-Header for using op-amp
 * 04-13-2023 E. Brombaugh
 */

#ifndef _OPAMP_H
#define _OPAMP_H

#include <stdint.h>

/*
 * initialize adc for polling
 */
void opamp_init( void )
{
	// turn on the op-amp
	EXTEN->EXTEN_CTR |= EXTEN_OPA_EN;
	
	// select op-amp pos pin: 0 = PA2, 1 = PD7
	//EXTEN->EXTEN_CTR |= EXTEN_OPA_PSEL;

	// select op-amp neg pin: 0 = PA1, 1 = PD0
	//EXTEN->EXTEN_CTR |= EXTEN_OPA_PSEL;

}
#endif

