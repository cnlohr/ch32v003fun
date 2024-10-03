SEARCH_DIR(CH32V003FUNDIR)

MEMORY
{
#if TARGET_MCU_LD == 0
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 16K
	RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 2K
#elif TARGET_MCU_LD == 1
	#if MCU_PACKAGE == 1
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
	#elif MCU_PACKAGE == 2
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 32K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 10K
	#else
		#error "Unknown MCU package"
	#endif
#elif TARGET_MCU_LD == 2
	#if MCU_PACKAGE == 1
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 64K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
	#elif MCU_PACKAGE == 2
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 32K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 10K
	#elif MCU_PACKAGE == 3
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
	#else
		#error "Unknown MCU package"
	#endif
#elif TARGET_MCU_LD == 3
	#if MCU_PACKAGE == 1
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
	#elif MCU_PACKAGE == 2
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 32K
	#else
		#error "Unknown MCU package"
	#endif
#elif TARGET_MCU_LD == 4
	#if MCU_PACKAGE == 1
		FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 62K
		RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
	#else
		#error "Unknown MCU package"
	#endif
#else
	#error "Unknown MCU target"
#endif
}

INCLUDE "ch32v003fun_common.ld"