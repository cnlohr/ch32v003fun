TARGET_MCU_PACKAGE?=CH32V203F8P6
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
	-DCH32V20x=1 \
	-I. -Wall

# MCU Flash/RAM split
ifeq ($(findstring F8, $(TARGET_MCU_PACKAGE)), F8)
	MCU_PACKAGE:=1
else ifeq ($(findstring G8, $(TARGET_MCU_PACKAGE)), G8)
	MCU_PACKAGE:=1
else ifeq ($(findstring K8, $(TARGET_MCU_PACKAGE)), K8)
	MCU_PACKAGE:=1
else ifeq ($(findstring C8, $(TARGET_MCU_PACKAGE)), C8)
	MCU_PACKAGE:=1
else ifeq ($(findstring F6, $(TARGET_MCU_PACKAGE)), F6)
	MCU_PACKAGE:=2
else ifeq ($(findstring G6, $(TARGET_MCU_PACKAGE)), G6)
	MCU_PACKAGE:=2
else ifeq ($(findstring K6, $(TARGET_MCU_PACKAGE)), K6)
	MCU_PACKAGE:=2
else ifeq ($(findstring C6, $(TARGET_MCU_PACKAGE)), C6)
	MCU_PACKAGE:=2
else ifeq ($(findstring RB, $(TARGET_MCU_PACKAGE)), RB)
	MCU_PACKAGE:=3
else ifeq ($(findstring GB, $(TARGET_MCU_PACKAGE)), GB)
    MCU_PACKAGE:=3
else ifeq ($(findstring CB, $(TARGET_MCU_PACKAGE)), CB)
    MCU_PACKAGE:=3
else ifeq ($(findstring WB, $(TARGET_MCU_PACKAGE)), WB)
    MCU_PACKAGE:=3
endif

# Package
ifeq ($(findstring 203RB, $(TARGET_MCU_PACKAGE)), 203RB)
	CFLAGS+=-DCH32V20x_D8
else ifeq ($(findstring 208, $(TARGET_MCU_PACKAGE)), 208)
	CFLAGS+=-DCH32V20x_D8W
else
	CFLAGS+=-DCH32V20x_D6
endif

GENERATED_LD_FILE:=$(CH32V003FUN)/generated_$(TARGET_MCU_PACKAGE).ld
LINKER_SCRIPT:=$(GENERATED_LD_FILE)
FILES_TO_COMPILE:=$(SYSTEM_C) $(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)

$(GENERATED_LD_FILE) :
	$(PREFIX)-gcc -E -P -x c -DMCU_PACKAGE=$(MCU_PACKAGE) $(CH32V003FUN)/ch32v20xfun.ld > $(GENERATED_LD_FILE)

$(TARGET).elf : $(GENERATED_LD_FILE) $(FILES_TO_COMPILE)
	echo $(FILES_TO_COMPILE)
	$(PREFIX)-gcc -o $@ $(FILES_TO_COMPILE) $(CFLAGS) $(LDFLAGS)