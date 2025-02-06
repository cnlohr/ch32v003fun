all : flash

TARGET:=interrupt_timing_test
TARGET_MCU:=CH32V305
TARGET_MCU_PACKAGE:=CH32V305RBT6

CFLAGS+= -Wno-volatile-register-var -ffixed-s6 -ffixed-s7 -ffixed-s8 -ffixed-s9 -ffixed-s10 -ffixed-s11

include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean


