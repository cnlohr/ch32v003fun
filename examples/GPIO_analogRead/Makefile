TARGET:=GPIO_analogRead

CFLAGS+=-DTINYVECTOR
ADDITIONAL_C_FILES+=wiring.c

include ../../ch32v003fun/ch32v003fun.mk

all : flash
flash : cv_flash
clean : cv_clean

