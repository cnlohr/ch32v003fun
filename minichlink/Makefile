TOOLS:=wch_erase wch_reset wch_write_simple minichlink

all : $(TOOLS)

CFLAGS:=-O1 -g
LDFLAGS:=-lpthread -lusb-1.0

wch_erase : wch_erase.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)
wch_reset : wch_reset.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)
wch_write_simple : wch_write_simple.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)
minichlink : minichlink.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

install_udev_rules :
	echo "SUBSYSTEMS==\"usb\", ATTRS{idVendor}==\"1a86\", ATTRS{idProduct}==\"8010\", GROUP=\"plugdev\", MODE=\"0666\"" > /etc/udev/rules.d/99-qch-LinkE.rules

clean :
	rm -rf $(TOOLS)
	


