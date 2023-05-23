TOOLS:=minichlink minichlink.so

CFLAGS:=-O0 -g3 -Wall
C_S:=minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c minichgdb.c

# General Note: To use with GDB, gdb-multiarch
# gdb-multilib {file}
# target remote :2345

ifeq ($(OS),Windows_NT)
	LDFLAGS:=-L. -lpthread -lusb-1.0 -lsetupapi -lws2_32
	CFLAGS:=-Os -s -Wall
	TOOLS:=minichlink.exe
else
	OS_NAME := $(shell uname -s | tr A-Z a-z)
	ifeq ($(OS_NAME),linux)
		LDFLAGS:=-lpthread -lusb-1.0 -ludev
	endif
	ifeq ($(OS_NAME),darwin)
		LDFLAGS:=-lpthread -lusb-1.0 -framework CoreFoundation -framework IOKit
		CFLAGS:=-O0 -Wall -Wno-asm-operand-widths -Wno-deprecated-declarations -Wno-deprecated-non-prototype -D__MACOSX__
		INCLUDES:=$(shell pkg-config --cflags-only-I libusb-1.0)
		LIBINCLUDES:=$(shell pkg-config --libs-only-L libusb-1.0)
		INCS:=$(INCLUDES) $(LIBINCLUDES)
	endif
endif

all : $(TOOLS)

# will need mingw-w64-x86-64-dev gcc-mingw-w64-x86-64
minichlink.exe : $(C_S)
	x86_64-w64-mingw32-gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

minichlink : $(C_S)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS)

minichlink.so : $(C_S)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS) -shared -fPIC

minichlink.dll : $(C_S)
	x86_64-w64-mingw32-gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS) -shared -DMINICHLINK_AS_LIBRARY

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

inspect_bootloader : minichlink
	./minichlink -r test.bin launcher 0x780
	riscv64-unknown-elf-objdump -S -D test.bin -b binary -m riscv:rv32 | less

clean :
	rm -rf $(TOOLS) 
