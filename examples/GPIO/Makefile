all : flash

TARGET:=GPIO

CFLAGS+=-DTINYVECTOR
ADDITIONAL_C_FILES+=../../extralibs/wiring.c

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

