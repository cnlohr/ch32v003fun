all : flash

TARGET:=flash_storage_optionbytes
CFLAGS+=-DTINYVECTOR -DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean


