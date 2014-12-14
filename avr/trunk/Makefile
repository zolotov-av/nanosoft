
MCU = atmega16a

CC_OPTS = -mmcu=atmega16a -ffunction-sections -fno-exceptions
LD_OPTS = -mmcu=atmega16a -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar

OBJECTS=main.o
OBJECTS+=iom16a_usart.o
OBJECTS+=iom16a_spi.o

test.hex: test.o
	$(OBJCOPY) -O ihex test.o test.hex

test.o: main.o libnano.a iom16a_usart.o iom16a_spi.o
	$(GCC) $(LD_OPTS) $(OBJECTS) -o test.o -Wl,-L=. -lnano
	$(OBJDUMP) -d test.o > test.dump

main.o: main.cpp pictl.h iom16a_timer.h iom16a_usart.h iom16a_spi.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.cpp -o main.o
	$(OBJDUMP) -d main.o > main.dump

libnano.a: pictl.o http.o
	$(AR) rcs libnano.a pictl.o http.o

pictl.o: pictl.cpp pictl.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=pictl.s -O2 -c pictl.cpp -o pictl.o

http.o: http.cpp http.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=http.s -O2 -c http.cpp -o http.o

iom16a_usart.o: iom16a_usart.cpp iom16a_usart.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_usart.s -O2 -c iom16a_usart.cpp -o iom16a_usart.o

iom16a_spi.o: iom16a_spi.cpp iom16a_spi.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=iom16a_spi.s -O2 -c iom16a_spi.cpp -o iom16a_spi.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
