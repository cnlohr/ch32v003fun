all : flash

TARGET:=bootload
LINKER_SCRIPT:=../../ch32v003fun/ch32v003fun-bootloader.ld
WRITE_SECTION:=bootloader
FLASH_COMMAND:=../../minichlink/minichlink -a -U -w $(TARGET).bin $(WRITE_SECTION) -B

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

