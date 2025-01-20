all : flash

TARGET:=tim1_phase_shifted_pwm

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

# force PD7 to be GPIO instead of NRST
setup: 
	$(MINICHLINK)/minichlink -D