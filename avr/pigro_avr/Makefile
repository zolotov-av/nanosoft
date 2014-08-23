
MCU = atmega16a

CC_OPTS = -mmcu=atmega16a -ffunction-sections
LD_OPTS = -mmcu=atmega16a -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar


pigro_avr.hex: pigro_avr.o
	$(OBJCOPY) -O ihex pigro_avr.o pigro_avr.hex

pigro_avr.o: main.o iom16a_usart.o iom16a_spi.o
	$(GCC) $(LD_OPTS) main.o iom16a_usart.o iom16a_spi.o -o pigro_avr.o
	$(OBJDUMP) -d pigro_avr.o > pigro_avr.dump

main.o: main.c iom16a_usart.h iom16a_spi.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.c -o main.o
	$(OBJDUMP) -d main.o > main.dump

iom16a_usart.o: iom16a_usart.c iom16a_usart.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_usart.s -O2 -c iom16a_usart.c -o iom16a_usart.o

iom16a_spi.o: iom16a_spi.c iom16a_spi.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_spi.s -O2 -c iom16a_spi.c -o iom16a_spi.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
