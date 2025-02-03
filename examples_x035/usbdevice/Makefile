all : flash

TARGET:=usbdevice
TARGET_MCU:=CH32X035
ADDITIONAL_C_FILES:=
include ../../ch32fun/ch32fun.mk

install_udev_rules : 
	sudo cp 99-x035demo.rules /etc/udev/rules.d/
	sudo udevadm control --reload
	sudo udevadm trigger


flash : cv_flash
clean : cv_clean

