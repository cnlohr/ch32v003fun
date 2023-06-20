// DOES NOT WORK HALP!!!!!!!!!!!!!!

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	Delay_Ms(100);

	printf( "Starting\n" );

	// Unkock flash - be aware you need extra stuff for the bootloader.
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;

	// For option bytes.
//	FLASH->OBKEYR = 0x45670123;
//	FLASH->OBKEYR = 0xCDEF89AB;

	FLASH->MODEKEYR = 0x45670123;
	FLASH->MODEKEYR = 0xCDEF89AB;

	printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );
	if( FLASH->CTLR & 0x8080 ) 
	{
		printf( "Flash still locked\n" );
		while(1);
	}

	uint32_t * ptr = (uint32_t*)0x08002700;
	printf( "Memory at: 0x08002710: %08lx %08lx\n", ptr[0], ptr[1] );

	//Erase Page
	FLASH->CTLR = CR_PAGE_ER;
	FLASH->ADDR = (intptr_t)ptr;
	FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
	printf( "FLASH->STATR = %08lx %08x\n", FLASH->STATR, FLASH_STATR_BSY );
	while( FLASH->STATR & FLASH_STATR_BSY );
	printf( "Erase complete\n" );


	// Clear buffer and prep for flashing.
	FLASH->CTLR = CR_PAGE_PG;
	FLASH->CTLR = CR_BUF_RST | CR_PAGE_PG;
	// Note: It takes about 8 clock cycles for this to finish.
	while( FLASH->STATR & FLASH_STATR_BSY );

	printf( "Memory at: 0x08002710: %08lx %08lx\n", ptr[0], ptr[1] );

	int i;
	for( i = 0; i < 16; i++ )
	{
		ptr[i] = 0xabcd1234; //Write to the memory
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		printf( "(loop) FLASH->STATR = %08lx\n", FLASH->STATR );
		while( FLASH->STATR & FLASH_STATR_BSY );
	}

	FLASH->ADDR = (intptr_t)ptr;
	FLASH->CTLR = CR_PAGE_PG|CR_STRT_Set;
	printf( "FLASH->STATR = %08lx\n", FLASH->STATR );
	while( FLASH->STATR & FLASH_STATR_BSY );

	printf( "Memory at: 0x08002710: %08lx %08lx\n", ptr[0], ptr[1] );

	while(1);
}

