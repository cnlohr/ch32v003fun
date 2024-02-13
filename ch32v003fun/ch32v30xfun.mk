TARGET_MCU_PACKAGE?=CH32V307VCT6
MCU_PACKAGE?=1

CFLAGS+= \
	-g -Os -flto -ffunction-sections -fdata-sections \
	-static-libgcc \
	-march=rv32imafc \
	-mabi=ilp32f \
	-msmall-data-limit=8 \
	-mno-save-restore \
	-fmessage-length=0 \
	-fsigned-char \
	-I/usr/include/newlib \
	-I$(CH32V003FUN) \
	-nostdlib \
	-DCH32V30x=1 \
	-I. -Wall

# MCU Flash/RAM split
ifeq ($(findstring RC, $(TARGET_MCU_PACKAGE)), RC)
	MCU_PACKAGE:=1
else ifeq ($(findstring VC, $(TARGET_MCU_PACKAGE)), VC)
	MCU_PACKAGE:=1
else ifeq ($(findstring WC, $(TARGET_MCU_PACKAGE)), WC)
	MCU_PACKAGE:=1
else ifeq ($(findstring CB, $(TARGET_MCU_PACKAGE)), CB)
	MCU_PACKAGE:=2
else ifeq ($(findstring FB, $(TARGET_MCU_PACKAGE)), FB)
	MCU_PACKAGE:=2
else ifeq ($(findstring RB, $(TARGET_MCU_PACKAGE)), RB)
	MCU_PACKAGE:=2
endif

# Package
ifeq ($(findstring 303, $(TARGET_MCU_PACKAGE)), 303)
	CFLAGS+=-DCH32V30x_D8
else
	CFLAGS+=-DCH32V30x_D8C
endif

TARGET_MCU_LD:=3
LDFLAGS+=-lgcc

GENERATED_LD_FILE:=$(CH32V003FUN)/generated_$(TARGET_MCU_PACKAGE).ld
LINKER_SCRIPT:=$(GENERATED_LD_FILE)
FILES_TO_COMPILE:=$(SYSTEM_C) $(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)

