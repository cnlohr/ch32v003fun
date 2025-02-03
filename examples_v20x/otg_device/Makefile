all : flash

TARGET:=otg_device
TARGET_MCU:=CH32V203
ADDITIONAL_C_FILES:=
include ../../ch32fun/ch32fun.mk

install_udev_rules : 
	sudo cp ../../examples_v20x/otg_device/99-v203demo.rules /etc/udev/rules.d/
	sudo udevadm control --reload
	sudo udevadm trigger


flash : cv_flash
clean : cv_clean

