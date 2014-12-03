
MCU = attiny13

CC_OPTS = -mmcu=attiny13 -ffunction-sections
LD_OPTS = -mmcu=attiny13 -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar


test.hex: test.o
	$(OBJCOPY) -O ihex test.o test.hex

test.o: main.o
	$(GCC) $(LD_OPTS) main.o -o test.o
	$(OBJDUMP) -d test.o > test.dump

main.o: main.c iot13_timer.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.c -o main.o
	$(OBJDUMP) -d main.o > main.dump

#iom16a_usart.o: iom16a_usart.c iom16a_usart.h
#	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_usart.s -O2 -c iom16a_usart.c -o iom16a_usart.o
#
#iom16a_spi.o: iom16a_spi.c iom16a_spi.h
#	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_spi.s -O2 -c iom16a_spi.c -o iom16a_spi.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
