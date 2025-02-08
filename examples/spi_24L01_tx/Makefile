all : flash

TARGET:=spi_24L01_tx
ADDITIONAL_C_FILES+=nrf24l01_low_level.c nrf24l01.c 

TARGET_MCU?=CH32V003
include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean
