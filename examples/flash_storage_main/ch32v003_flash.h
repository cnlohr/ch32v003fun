//######## necessities

// include guards
#ifndef CH32V003_FLASH_H
#define CH32V003_FLASH_H

// includes
#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"



/*######## library usage and configuration
HARDWARE:
Integrated flash has limited write cycles (10K as per datasheet) and will thus wear out quickly if you're not very careful!!
For frequent writes use external flash chips, they are far more robust!
CH32V003 has 16K =
	16 * 2^10 bytes
	16384 bytes
	0x08003FFF-0x08000000+1
	256 pages * 64 bytes
erase	= turn all bits into 1
program = turn the desired bits into 0, always 2 bytes at a time
write	= erase + program



MAIN GOAL
ability to
	store settings in flash (non-volatile)
	retrieve settings from flash after power cycle

IMPLEMENTATION:
write to and read from
	main flash
		reserved page(s) at the end (64 bytes)
	option bytes data1 and data0
		for some applications 16 bits are enough



USAGE:
to store variables in main flash you need to reserve 1 page or more at the end of main flash for storing non-volatile variables:

in overrides.ld
PROVIDE(FLASH_LENGTH_OVERRIDE = N_BYTES);
BYTES = 16 * 2^10 - 64 * pages
example, reserve 1 page:
PROVIDE(FLASH_LENGTH_OVERRIDE = 16320);

in your Makefile, before "include ../../ch32v003fun/ch32v003fun.mk", inject your linker file:
LDFLAGS+=-T overrides.ld 

#define SYSTEM_CORE_CLOCK with a value before #including this library 
During your boot phase, call flash_set_latency() once.

Functions without "OB" in the name concern the main flash.
"alter" is erase / program / write
Read-operations (the getter functions) can be performed at any time and do not require unlocking.

To alter main flash:
1. unlock flash
2. erase page(s) (sadly we can't erase any smaller)
3. program all desired values to the pages
4. lock

To alter option bytes data1 and data0:
1. unlock flash
2. unlock OB
3. write. this will internally erase them and restore all other values of the option bytes as they were before.
4. lock



TECHNICAL NOTES:

main flash:
trying to write to addresses outside the main flash (offsets larger than what you reserved) will likely make your micro controller freeze, so be sure that your code doesn't attempt.

option bytes (user-selected words):
The data is stored as IIIIIIII DDDDDDDD, where D is data (byte0) and I is the inverse of data (byte 1).
The user is supposed to be able to compare and discover errors, but it doesn't allow for error recovery.
To write a byte (8 bits), the write needs to be 16 bits (uint16_t), the upper 16 bits (I) will automagically be replaced with the inverted bit pattern of D.
To read them (and write the data1 and data0 bytes), feel free to use the provided functions, they abstract this away.
You may also OB->Data1 and discard the upper 8 (inverted) bits by >>8 yourself if you require raw speed.



ADDRESS CALCULATIONS:

for storing variables in main flash the address can be calculated like this:
address of byte nonvolatile[n] = FLASH_BASE + N_BYTES + [n];
where n is the offset from the start of your reserved nonvolatile storage
example:
nonvolatile[15] = 0x08000000 + 16320 + 1 + 15

flash_calcualte_nonvolatile_addr(n) does exactly that
suggestion: calculate all non-volatile storage adresses at beginning of main and store them in variables
better suggestion: use the FLASH_PRECALCULATE_NONVOLATILE_ADDR(n) preprocessor macro #defining your addresses, this way the math will be done at compile time
*/



//######## function overview (declarations): use these!

// calculate address at runtime, you can use compile-time addresses instead!
static inline uint32_t flash_calcualte_nonvolatile_addr(uint16_t byte_number);

// set the flash controller latency in accordance with the SYSTEM_CORE_CLOCK speed
static inline void flash_set_latency();

// unlock flash altering
static inline void flash_unlock();
// unlock option bytes altering, additionally
static inline void flash_OB_unlock();
// lock flash when you're done
static inline void flash_lock();

// erase a page (sorry, smaller erases impossible on CH32V003!)
// x -> 1
static inline void flash_erase_64b(uint32_t start_addr);

// program 2 bytes at once
// 1 -> 0
static inline void flash_program_16(uint32_t addr, uint16_t data);
static inline void flash_program_2x8(uint32_t addr, uint8_t byte1, uint8_t byte0);
static inline void flash_program_float(uint32_t addr, float value);

// read data
static inline uint16_t flash_get_16(uint32_t addr);
static inline uint8_t flash_get_8(uint32_t addr);
static inline float flash_get_float(uint32_t addr);

// write data to option bytes, keeping other contents intact
// 		= backup + erase + write (restoring backup)
static inline void flash_OB_write_data_16(uint16_t data);
static inline void flash_OB_write_data_2x8(uint8_t data1, uint8_t data0);

// read option bytes
static inline uint8_t flash_OB_get_USER();
static inline uint8_t flash_OB_get_RDPR();
static inline uint8_t flash_OB_get_WRPR1();
static inline uint8_t flash_OB_get_WRPR0();

// read data of option bytes
static inline uint8_t flash_OB_get_DATA1();
static inline uint8_t flash_OB_get_DATA0();
static inline uint16_t flash_OB_get_DATA_16();



//######## internal function declarations

// get opterational states of flash
static inline uint8_t flash_is_busy();
static inline uint8_t flash_is_done();
static inline uint8_t flash_is_ERR_WRPRT();

// clear flash operation done (EOP) bit
static inline void flash_is_done_clear();

// wait for flash
static inline void flash_wait_until_not_busy();
static inline void flash_wait_until_done();

// test that first 8 bits are the inverse of the last 8 bits
static inline uint8_t flash_dechecksum(uint16_t input);

// erase option byte
static inline void flash_OB_erase();



//######## internal variables

extern char FLASH_LENGTH_OVERRIDE[]; // import from .ld, halal by https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html

union float_2xuint16t {
	float f;
	uint16_t u16[2];
};

//######## preprocessor macros

#define FLASH_VOLATILE_CAPACITY (FLASH_BASE-FLASH_LENGTH_OVERRIDE)

// use this to define main flash nonvolatile addresses at compile time!
#define FLASH_PRECALCULATE_NONVOLATILE_ADDR(n) FLASH_BASE+(uint32_t)(uintptr_t)(FLASH_LENGTH_OVERRIDE)+n 



//######## preprocessor #define requirements


#ifndef SYSTEM_CORE_CLOCK
#error "SYSTEM_CORE_CLOCK is not defined. Please define it in your .c before you #include ch32v003_flash.h."
#endif


//######## small function definitions, static inline

static inline uint32_t flash_calcualte_nonvolatile_addr(uint16_t byte_number) {
	//if (byte_number < FLASH_VOLATILE_CAPACITY) {}
	return (FLASH_BASE + (uint16_t)(uintptr_t)FLASH_LENGTH_OVERRIDE + byte_number);
}

static inline void flash_set_latency() {
	#if SYSTEM_CORE_CLOCK <= 24000000
		FLASH->ACTLR = FLASH_Latency_0;
	#else
		FLASH->ACTLR = FLASH_Latency_1;
	#endif
}

static inline void flash_unlock() {
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
}
static inline void flash_OB_unlock() {
	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;
}
static inline void flash_lock() {
	FLASH->CTLR |= FLASH_CTLR_LOCK;
}

// x -> 1
static inline void flash_erase_64b(uint32_t start_addr) {
	if(FLASH->CTLR & FLASH_CTLR_LOCK) {
		return;
	}
	flash_wait_until_not_busy();
	FLASH->CTLR |= CR_PER_Set;
	FLASH->ADDR = start_addr; 
	FLASH->CTLR |= CR_STRT_Set;
	flash_wait_until_not_busy();
	FLASH->CTLR &= CR_PER_Reset;
}

// 1 -> 0
static inline void flash_program_16(uint32_t addr, uint16_t data) {
	if(FLASH->CTLR & FLASH_CTLR_LOCK) {
		return;
	}
	flash_wait_until_not_busy();
	//FLASH->ADDR = addr;
	//FLASH->OBR = 0;
	//FLASH->OBR |= data << 10;
	FLASH->CTLR |= CR_PG_Set;
	*(uint16_t*)(uintptr_t)addr = data;
	flash_wait_until_not_busy();
	//flash_wait_until_done();
	FLASH->CTLR &= CR_PG_Reset;
}
static inline void flash_program_2x8(uint32_t addr, uint8_t byte1, uint8_t byte0) {
	flash_program_16(addr, (byte1<<8)+byte0);
}
static inline void flash_program_float(uint32_t addr, float value) {
	union float_2xuint16t conv;
	conv.f = value;
	flash_program_16(addr, conv.u16[0]);
	flash_program_16(addr, conv.u16[1]);
}

static inline uint16_t flash_get_16(uint32_t addr) {
	return *(uint16_t*)(uintptr_t)addr;
}
static inline uint8_t flash_get_8(uint32_t addr) {
	return *(uint8_t*)(uintptr_t)addr;
}
static inline float flash_get_float(uint32_t addr) {
	union float_2xuint16t conv;
	conv.u16[0] = ((uint16_t*)(uintptr_t)addr)[0];
	conv.u16[1] = ((uint16_t*)(uintptr_t)addr)[1];
	return conv.f;
}
/*
 * proof I don't know how to do bitmath on floats
static inline float flash_program_float(uint32_t addr, float value) {
	flash_program_16(addr, (value & 0b1111111111111111));
	flash_program_16(addr, ((value >> 16) & 0b1111111111111111));
}
static inline float flash_get_float(uint32_t addr) {
	return *((float*)(uintptr_t)(addr));
}
*/

// 1 -> 0
// is inlining still ok?
//		big
//		but won't get called often / from many places
//	else move to implementation block and remove static inline
static inline void flash_OB_write_data_16(uint16_t data) {
	flash_wait_until_not_busy();
	//backup
	uint16_t tmp_user = OB->USER; 
	uint16_t tmp_rdpr = OB->RDPR; 
	uint16_t tmp_wrpr1 = OB->WRPR1; 
	uint16_t tmp_wrpr0 = OB->WRPR0; 
	uint16_t tmp_data1 = (data>>8) & 0b11111111;
	uint16_t tmp_data0 = data & 0b11111111;

	//erase
	flash_OB_erase();

	//enable programming
	FLASH->CTLR |= CR_OPTPG_Set;

	//restore backup
	OB->USER = tmp_user; 
	flash_wait_until_not_busy();
	OB->RDPR = tmp_rdpr;
	flash_wait_until_not_busy();
	OB->WRPR0 = tmp_wrpr0;
	flash_wait_until_not_busy();
	OB->WRPR1 = tmp_wrpr1;
	flash_wait_until_not_busy();

	// write data
	OB->Data1 = tmp_data1;				// data high byte
	flash_wait_until_not_busy();
	OB->Data0 = tmp_data0;				// data low byte
	flash_wait_until_not_busy();
	
	//disable programming
	FLASH->CTLR &= CR_OPTPG_Reset;
}
static inline void flash_OB_write_data_2x8(uint8_t data1, uint8_t data0) {
	flash_OB_write_data_16((data1<<8)+data0);
}

static inline uint8_t flash_OB_get_USER() {
	return flash_dechecksum(OB->USER);
}
static inline uint8_t flash_OB_get_RDPR() {
	return flash_dechecksum(OB->RDPR);
}
static inline uint8_t flash_OB_get_WRPR1() {
	return flash_dechecksum(OB->WRPR1);
}
static inline uint8_t flash_OB_get_WRPR0() {
	return flash_dechecksum(OB->WRPR0);
}
static inline uint8_t flash_OB_get_DATA1() {
	return flash_dechecksum(OB->Data1);
}
static inline uint8_t flash_OB_get_DATA0() {
	return flash_dechecksum(OB->Data0);
}
static inline uint16_t flash_OB_get_DATA_16() {
	return (flash_OB_get_DATA1()<<8)+flash_OB_get_DATA0();
}



//######## small internal function definitions, static inline
static inline uint8_t flash_is_busy() {
	return ((FLASH->STATR & FLASH_STATR_BSY) == FLASH_STATR_BSY);
}
static inline uint8_t flash_is_done() {
	return ((FLASH->STATR & FLASH_STATR_EOP) == FLASH_STATR_EOP);
}
static inline uint8_t flash_is_ERR_WRPRT() {
	return ((FLASH->STATR & FLASH_STATR_WRPRTERR) == FLASH_STATR_WRPRTERR);
}
static inline void flash_is_done_clear() {
	FLASH->STATR |= FLASH_STATR_EOP;
}

static inline void flash_wait_until_not_busy() {
	while(flash_is_busy()) {}
}
static inline void flash_wait_until_done() {
	while(flash_is_busy() || !flash_is_done()) {}
	flash_is_done_clear();
}

static inline uint8_t flash_dechecksum(uint16_t input) {
	uint8_t noninverted = input & 0b11111111;
	uint8_t deinverted = ~(input>>8);
	if (deinverted == noninverted) {
		return noninverted;
	}
	else {
		return 0;
	}
}

// x -> 1
static inline void flash_OB_erase() {
	FLASH->CTLR |= CR_OPTER_Set;
	FLASH->CTLR |= CR_STRT_Set;
	flash_wait_until_not_busy();
	FLASH->CTLR &= CR_OPTER_Reset;
}



//######## implementation block
//#define CH32V003_FLASH_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_FLASH_IMPLEMENTATION)



#endif // CH32V003_FLASH_IMPLEMENTATION
#endif // CH32V003_FLASH_H
