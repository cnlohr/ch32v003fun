all : flash

TARGET:=exti_pin_change_isr

test : flash
	../../minichlink/minichlink -X ECLK:1:0:0:1:63

TARGET_MCU?=CH32V003
include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean


