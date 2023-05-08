all : flash

TARGET:=spi_oled

CFLAGS+=-I../i2c_oled
#CFLAGS+=-DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

