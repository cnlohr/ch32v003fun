//
// Example code for writing to the two 8-bit data0/data1 bytes that are
// a part of the Flash "User-selected words" (aka Option bytes).
// To reduce the code footpint no timeouts are implemented in the code. It simply
// waits for the busy-flag to be cleaed. The two wait-loops could be replaced 
// with a Delay_Ms(10) if speed is not an issue.
//
// June 13, 2023 Mats Engstrom (github.com/mengstr)
//

#include "ch32v003fun.h"
#include <stdio.h>
void FlashOptionData(uint8_t data0, uint8_t data1) {
	volatile uint16_t hold[6]; 		// array to hold current values while erasing

	// The entire 64 byte data block of the "User-selected words" will be erased
	// so we need to keep a copy of the content for re-writing after erase.
	// Save a few (20) bytes code space by moving 32 bits at a time.
	// 			hold[0]=OB->RDPR;
	// 			hold[1]=OB->USER;
	// 			hold[2]=data0;
	// 			hold[3]=data1;
	// 			hold[4]=OB->WRPR0;
	// 			hold[5]=OB->WRPR1;
	uint32_t *hold32p=(uint32_t *)hold;
	uint32_t *ob32p=(uint32_t *)OB_BASE;
	hold32p[0]=ob32p[0]; 			// Copy RDPR and USER
	hold32p[1]=data0+(data1<<16);	// Copy in the two Data values to be written
	hold32p[2]=ob32p[2];			// Copy WRPR0 and WEPR1

	// Unlock both the general Flash and the User-selected words
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;

	FLASH->CTLR |= CR_OPTER_Set;			// OBER RW Perform user-selected word erasure	
	FLASH->CTLR |= CR_STRT_Set;    			// STRT RW1 Start. Set 1 to start an erase action,hw automatically clears to 0
	while (FLASH->STATR & FLASH_BUSY);		// Wait for flash operation to be done
	FLASH->CTLR &= CR_OPTER_Reset; 			// Disable erasure mode	

	// Write the held values back one-by-one
	FLASH->CTLR |= CR_OPTPG_Set;   			// OBG  RW Perform user-selected word programming
	uint16_t *ob16p=(uint16_t *)OB_BASE;
	for (int i=0;i<sizeof(hold)/sizeof(hold[0]); i++) {
		ob16p[i]=hold[i];
		while (FLASH->STATR & FLASH_BUSY);	// Wait for flash operation to be done
	}
	FLASH->CTLR &= CR_OPTPG_Reset;			// Disable programming mode

	FLASH->CTLR|=CR_LOCK_Set;				// Lock flash memories again

	return;
}



//
//
//
int main() {
	SystemInit();
	Delay_Ms(250);
	
	uint8_t bootcnt=OB->Data0;
	bootcnt++;
	FlashOptionData(bootcnt,0);
	printf("Boot count is %d\n",bootcnt);

	for(;;);
}
