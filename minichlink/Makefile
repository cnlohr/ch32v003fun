TOOLS:=minichlink

all : $(TOOLS)

CFLAGS:=-O0 -g3
LDFLAGS:=-lpthread -lusb-1.0 -ludev

minichlink : minichlink.c pgm-wch-linke.c pgm-esp32s2-ch32xx.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

clean :
	rm -rf $(TOOLS)
	


