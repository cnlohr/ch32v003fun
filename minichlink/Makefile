TOOLS:=minichlink minichlink.so

all : $(TOOLS)

CFLAGS:=-O0 -g3 -Wall

ifeq ($(OS),Windows_NT)
	LDFLAGS:=-lpthread -lusb-1.0 -lsetupapi
else
	OS_NAME := $(shell uname -s | tr A-Z a-z)
	ifeq ($(OS_NAME),linux)
		LDFLAGS:=-lpthread -lusb-1.0 -ludev
	endif
	ifeq ($(OS_NAME),darwin)
		LDFLAGS:=-lpthread -lusb-1.0 -framework CoreFoundation -framework IOKit
		CFLAGS:=-O0 -Wall -Wno-asm-operand-widths -Wno-deprecated-declarations -Wno-deprecated-non-prototype -D__MACOSX__
		INCLUDES:=-I /opt/homebrew/Cellar/libusb/1.0.26/include/libusb-1.0
		LIBINCLUDES:=-L /opt/homebrew/Cellar/libusb/1.0.26/lib 
		INCS:=$(INCLUDES) $(LIBINCLUDES)
	endif
endif




minichlink : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS)

minichlink.so : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS) -shared -fPIC

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

inspect_bootloader : minichlink
	./minichlink -r test.bin launcher 0x780
	riscv64-unknown-elf-objdump -S -D test.bin -b binary -m riscv:rv32 | less

clean :
	rm -rf $(TOOLS) 
