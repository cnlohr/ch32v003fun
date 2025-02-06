// Example testing memory option bytes on ch32v307
/* 
   WARNING Portions of this code are under the following copyright.
*/
/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file provides all the FLASH firmware functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

#ifdef TARGET_MCU_MEMORY_SPLIT
#define BUILD_MEMORY_SPLIT TARGET_MCU_MEMORY_SPLIT
#else
#define BUILD_MEMORY_SPLIT -1
#endif

int main()
{
	SystemInit();

	Delay_Ms( 100 );

	printf("Chip ID: %08x\n", INFO->CHIPID );
	printf("Device ID: %04x\n", INFO->DEVID );
	printf("Revision ID: %04x\n", INFO->REVID );

	printf("Option bytes in flash are: %04x\n", OB->USER );

	printf("Current RAM split %d\n", (uint8_t)((FLASH->OBR >> 8) & 0x3));
	printf("Flash RAM split %d\n", (uint8_t)((OB->USER >> 6) & 0x3));
	printf("Build RAM split %d\n", BUILD_MEMORY_SPLIT);

	// Now check the RAM capacity by sweeping a read and write back operation
	// in 4k steps until we trigger a fault
	printf("Testing RAM capacity:\n");

	// volatile to prevent the no-op from being optimized out
	volatile uint8_t *poke_addr = (void *) SRAM_BASE;

	for (int i = 0; i <= 32; i++) {
		uint32_t offset = 0x1000 * i;
		printf("%3dk test: Poking SRAM+0x%05lx... ", i, offset);
		poke_addr[offset] = poke_addr[offset];
		printf("success!\n");
	}

	while(1);
}