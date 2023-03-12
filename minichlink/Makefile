TOOLS:=minichlink

all : $(TOOLS)

CFLAGS:=-O0 -g3
LDFLAGS:=-lpthread -lusb-1.0 -ludev

minichlink : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

inspect_bootloader : minichlink
	./minichlink -r test.bin launcher 0x780
	riscv64-unknown-elf-objdump -S -D test.bin -b binary -m riscv:rv32 | less

clean :
	rm -rf $(TOOLS)
	


