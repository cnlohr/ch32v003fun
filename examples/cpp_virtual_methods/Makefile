all : flash

TARGET:=cpp_virtual_methods
TARGET_EXT:=cpp

ADDITIONAL_C_FILES+=example.cpp

include ../../ch32v003fun/ch32v003fun.mk

# Removing compiler optimization to small file size
# because it optimizes the virtual functions out
# which are tested here.
CFLAGS:= \
	-g -flto -ffunction-sections \
	-static-libgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-I$(CH32V003FUN) \
	-nostdlib \
	-I. -Wall

CFLAGS+=-fno-rtti -DSTDOUT_UART -DCPLUSPLUS

flash : cv_flash
clean : cv_clean