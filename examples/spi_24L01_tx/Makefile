all : flash

TARGET:=spi_24L01_tx
ADDITIONAL_C_FILES+=nrf24l01_low_level.c nrf24l01.c 

CFLAGS+=-DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean
