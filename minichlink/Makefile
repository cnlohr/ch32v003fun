TOOLS:=minichlink minichlink.so

all : $(TOOLS)

CFLAGS:=-O0 -g3 -Wall

C_S:=minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c minichgdb.c

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

# will need mingw-w64-x86-64-dev gcc-mingw-w64-x86-64
minichlink.exe : $(C_S)
	x86_64-w64-mingw32-gcc -o $@ $^ -Os -s -lsetupapi ./libusb-1.0.dll

minichlink : $(C_S)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS)

minichlink.so : $(C_S)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) $(INCS) -shared -fPIC

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

inspect_bootloader : minichlink
	./minichlink -r test.bin launcher 0x780
	riscv64-unknown-elf-objdump -S -D test.bin -b binary -m riscv:rv32 | less

clean :
	rm -rf $(TOOLS) 
