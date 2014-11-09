
CC_OPTS = -ffunction-sections
LD_OPTS = -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar

all: pigro_m8.hex pigro_avr.hex

pigro_m8.hex: pigro_m8.o
	$(OBJCOPY) -O ihex pigro_m8.o pigro_m8.hex

pigro_avr.hex: pigro_avr.o
	$(OBJCOPY) -O ihex pigro_avr.o pigro_avr.hex

pigro_m8.o: main_m8.o iom16a_usart.o iom8_spi.o
	$(GCC) -mmcu=atmega8 $(LD_OPTS) main_m8.o iom16a_usart.o iom8_spi.o -o pigro_m8.o
	$(OBJDUMP) -d pigro_m8.o > pigro_m8.dump

pigro_avr.o: main.o iom16a_usart.o iom16a_spi.o
	$(GCC) -mmcu=atmega16a $(LD_OPTS) main.o iom16a_usart.o iom16a_spi.o -o pigro_avr.o
	$(OBJDUMP) -d pigro_avr.o > pigro_avr.dump

main_m8.o: main_m8.c iom8_usart.h iom8_spi.h
	$(GCC) -mmcu=atmega8 $(CC_OPTS) $(ASM_LIST)=main_m8.s -O2 -c main_m8.c -o main_m8.o
	$(OBJDUMP) -d main_m8.o > main_m8.dump

main.o: main.c iom16a_usart.h iom16a_spi.h
	$(GCC) -mmcu=atmega16a $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.c -o main.o
	$(OBJDUMP) -d main.o > main.dump

iom8_usart.o: iom8_usart.c iom8_usart.h
	$(GCC) -mmcu=atmega8 $(CC_OPTS) $(ASM_LIST)=iom8_usart.s -O2 -c iom8_usart.c -o iom8_usart.o

iom16a_usart.o: iom16a_usart.c iom16a_usart.h
	$(GCC) -mmcu=atmega16a $(CC_OPTS) $(ASM_LIST)=iom16a_usart.s -O2 -c iom16a_usart.c -o iom16a_usart.o

iom8_spi.o: iom8_spi.c iom8_spi.h
	$(GCC) -mmcu=atmega8 $(CC_OPTS) $(ASM_LIST)=iom8_spi.s -O2 -c iom8_spi.c -o iom8_spi.o

iom16a_spi.o: iom16a_spi.c iom16a_spi.h
	$(GCC) -mmcu=atmega16a $(CC_OPTS) $(ASM_LIST)=iom16a_spi.s -O2 -c iom16a_spi.c -o iom16a_spi.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
