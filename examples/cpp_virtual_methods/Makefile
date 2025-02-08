all : flash

TARGET:=cpp_virtual_methods
TARGET_EXT:=cpp

ADDITIONAL_C_FILES+=example.cpp

TARGET_MCU?=CH32V003
include ../../ch32fun/ch32fun.mk

# Removing compiler optimization to small file size
# because it optimizes the virtual functions out
# which are tested here.
CFLAGS:= \
	-g -flto -ffunction-sections \
	-static-libgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I$(NEWLIB) \
	-I$(CH32FUN) \
	-nostdlib \
	-DCH32V003 \
	-I. -Wall

CFLAGS+=-fno-rtti -DCPLUSPLUS

flash : cv_flash
clean : cv_clean
