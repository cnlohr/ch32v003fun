all : flash

TARGET:=turbo_adc_with_usb
TARGET_MCU:=CH32V305
TARGET_MCU_PACKAGE:=CH32V305RBT6


ENABLE_FPU:=0

ADDITIONAL_C_FILES:=../../extralibs/hsusb_v30x.c
include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean

