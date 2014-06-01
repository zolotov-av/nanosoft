
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

test.o: main.o libfoo.a
	$(GCC) $(LD_OPTS) main.o -o test.o -Wl,-L=. -lfoo
	$(OBJDUMP) -d test.o > test.dump

main.o: main.c
	$(GCC) $(CC_OPTS) $(ASM_LIST)=main.s -O2 -c main.c -o main.o
	$(OBJDUMP) -d main.o > main.dump

libfoo.a: foo.o
	$(AR) rcs libfoo.a foo.o

foo.o: foo.c foo.h
	$(GCC) $(CC_OPTS) $(ASM_LIST)=foo.s -O2 -c foo.c -o foo.o

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
