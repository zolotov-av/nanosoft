
MCU = atmega16a

CC_OPTS = -mmcu=atmega16a -ffunction-sections
LD_OPTS = -mmcu=atmega16a -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar


test.hex: test.o
	$(OBJCOPY) -O ihex test.o test.hex

test.o: main.o libnano.a iom16a_usart.o
	$(GCC) $(LD_OPTS) main.o iom16a_usart.o -o test.o -Wl,-L=. -lnano
	$(OBJDUMP) -d test.o > test.dump

main.o: main.c pictl.h timers.h iom16a_usart.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.c -o main.o
	$(OBJDUMP) -d main.o > main.dump

libnano.a: pictl.o http.o
	$(AR) rcs libnano.a pictl.o http.o

pictl.o: pictl.c pictl.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=pictl.s -O2 -c pictl.c -o pictl.o

http.o: http.c http.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=http.s -O2 -c http.c -o http.o

iom16a_usart.o: iom16a_usart.c iom16a_usart.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_usart.s -O2 -c iom16a_usart.c -o iom16a_usart.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
