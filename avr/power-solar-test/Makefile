
MCU = attiny2313

CC_OPTS := -mmcu=$(MCU) -ffunction-sections -fno-exceptions
LD_OPTS := -mmcu=$(MCU) -Wl,--gc-sections
ASM_LIST = -Wa,-adhln

OBJECTS=main.o
#OBJECTS+=iom16a_usart.o
#OBJECTS+=iom16a_spi.o
#OBJECTS+=iom16a_adc.o

test.hex: test.o
	$(OBJCOPY) -O ihex test.o test.hex

test.o: $(OBJECTS)
	$(GCC) $(LD_OPTS) $(OBJECTS) -o test.o
	$(OBJDUMP) -d test.o > test.dump

main.o: main.cpp iom16a_timer.h
	$(GCC) $(CC_OPTS) -O2 -c main.cpp -o main.o
	$(OBJDUMP) -d main.o > main.dump

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
