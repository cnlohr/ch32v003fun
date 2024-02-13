TARGET_MCU_PACKAGE?=CH32V103R8T6
MCU_PACKAGE?=1

CFLAGS+= \
	-g -Os -flto -ffunction-sections -fdata-sections \
	-static-libgcc \
	-march=rv32imac \
	-mabi=ilp32 \
	-msmall-data-limit=8 \
	-mno-save-restore \
	-fmessage-length=0 \
	-fsigned-char \
	-I/usr/include/newlib \
	-I$(CH32V003FUN) \
	-nostdlib \
	-DCH32V10x=1 \
	-I. -Wall

# MCU Flash/RAM split
ifeq ($(findstring R8, $(TARGET_MCU_PACKAGE)), R8)
	MCU_PACKAGE:=1
else ifeq ($(findstring C8, $(TARGET_MCU_PACKAGE)), C8)
	MCU_PACKAGE:=1
else ifeq ($(findstring C6, $(TARGET_MCU_PACKAGE)), C6)
	MCU_PACKAGE:=2
endif

TARGET_MCU_LD:=1
LDFLAGS+=-lgcc

GENERATED_LD_FILE:=$(CH32V003FUN)/generated_$(TARGET_MCU_PACKAGE).ld
LINKER_SCRIPT:=$(GENERATED_LD_FILE)
FILES_TO_COMPILE:=$(SYSTEM_C) $(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)

