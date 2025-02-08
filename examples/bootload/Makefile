all : flash

TARGET:=bootload
LINKER_SCRIPT:=../../ch32fun/ch32v003fun-bootloader.ld
WRITE_SECTION:=bootloader
FLASH_COMMAND:=../../minichlink/minichlink -a -U -w $(TARGET).bin $(WRITE_SECTION) -B

TARGET_MCU?=CH32V003
include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean

