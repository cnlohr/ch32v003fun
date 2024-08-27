all : flash

TARGET:=usbdevice_hs
TARGET_MCU:=CH32V305
TARGET_MCU_PACKAGE:=CH32V305RBT6


ENABLE_FPU:=0

ADDITIONAL_C_FILES:=
include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

