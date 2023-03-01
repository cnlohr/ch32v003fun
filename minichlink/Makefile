TOOLS:=minichlink

all : $(TOOLS)

CFLAGS:=-O1 -g
LDFLAGS:=-lpthread -lusb-1.0

minichlink : minichlink.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

install_udev_rules :
	cp 99-WCH-LinkE.rules /etc/udev/rules.d/
	service udev restart

clean :
	rm -rf $(TOOLS)
	


