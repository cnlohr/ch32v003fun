TOOLS:=minichlink minichlink.so

all : $(TOOLS)

CFLAGS:=-O0 -g3 -Wall

ifeq ($(OS),Windows_NT)
	LDFLAGS:=-lpthread -lusb-1.0 -lsetupapi
else
	LDFLAGS:=-lpthread -lusb-1.0 -ludev
endif

minichlink : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

minichlink.so : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c nhc-link042.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS) -shared -fPIC

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

inspect_bootloader : minichlink
	./minichlink -r test.bin launcher 0x780
	riscv64-unknown-elf-objdump -S -D test.bin -b binary -m riscv:rv32 | less

clean :
	rm -rf $(TOOLS) 
