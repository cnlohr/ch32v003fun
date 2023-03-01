TOOLS:=minichlink

all : $(TOOLS)

CFLAGS:=-O1 -g
LDFLAGS:=-lpthread -lusb-1.0

minichlink : minichlink.c
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

install_udev_rules :
	echo "SUBSYSTEMS==\"usb\", ATTRS{idVendor}==\"1a86\", ATTRS{idProduct}==\"8010\", GROUP=\"plugdev\", MODE=\"0666\"" > /etc/udev/rules.d/99-qch-LinkE.rules
	service udev restart

clean :
	rm -rf $(TOOLS)
	


