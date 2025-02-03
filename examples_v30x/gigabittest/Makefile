all : flash

TARGET:=gigabittest
TARGET_MCU:=CH32V307
TARGET_MCU_PACKAGE:=CH32V307WCU6

include ../../ch32fun/ch32fun.mk

# memory split 0.
#optionbytes.dat : 
#	@/bin/echo -ne "\xa5\xa5\x3f\xc0\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00\xff\x00" > optionbytes.dat
#
#optionwrite : optionbytes.dat
#	$(MINICHLINK)/minichlink -w optionbytes.dat option

flash : cv_flash
clean : cv_clean
	rm -rf optionbytes.dat


