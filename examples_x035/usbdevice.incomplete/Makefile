all : flash

TARGET:=usbdevice
TARGET_MCU:=CH32X035
ADDITIONAL_C_FILES:=fsusb.c descriptor.c

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

