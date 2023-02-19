TARGET:=GPIO_Toggle

all : flash

PREFIX:=riscv64-unknown-elf

GPIO_Toggle:=EXAM/GPIO/GPIO_Toggle/User

CFLAGS:= \
	-g -Os -flto -ffunction-sections \
	-static-libgcc -lgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-nostdlib \
	-I$(GPIO_Toggle) \
	-IEXAM/SRC/Core \
	-IEXAM/SRC/Debug \
	-IEXAM/SRC/Debug \
	-IEXAM/SRC/Peripheral/inc

LDFLAGS:=-T EXAM/SRC/Ld/Link.ld

SYSTEM_C:=EXAM/SRC/Startup/startup_ch32v00x.S \
	EXAM/SRC/Debug/debug.c \
	EXAM/SRC/Peripheral/src/ch32v00x_gpio.c \
	EXAM/SRC/Peripheral/src/ch32v00x_rcc.c \
	EXAM/SRC/Peripheral/src/ch32v00x_usart.c \
	EXAM/SRC/Peripheral/src/ch32v00x_misc.c \
	../ch32v003evt/embedlibc.c

$(TARGET).elf : $(GPIO_Toggle)/ch32v00x_it.c $(GPIO_Toggle)/main.c $(GPIO_Toggle)/system_ch32v00x.c $(SYSTEM_C)
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).hex : GPIO_Toggle.elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	#$(PREFIX)-objcopy -O binary $< $@
	$(PREFIX)-objcopy -O ihex $< $@

	
flash : $(TARGET).hex

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).map

