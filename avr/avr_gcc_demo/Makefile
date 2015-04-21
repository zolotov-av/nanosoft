
# объявляем переменную
# например указываем MCU с которым мы будем работать
MCU := atmega16a

# Чтобы многократно не писать одно и тоже, вынесем общие опции в переменные
CFLAGS := -mmcu=$(MCU) -ffunction-sections -fno-exceptions
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

GCC_TARGET = avr
GCC = $(GCC_TARGET)-gcc
OBJCOPY = $(GCC_TARGET)-objcopy
OBJDUMP = $(GCC_TARGET)-objdump
AR = $(GCC_TARGET)-ar
ASM_LIST = -Wa,-adhln

# получение готовой прошивки в формате *.hex
test.hex: test.o
	avr-objcopy -O ihex test.o test.hex

# компонуем несколько объектных модулей в один объектный модуль
test.o: main.o mod_foo.o
	avr-gcc $(LDFLAGS) main.o mod_foo.o -o test.o
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d test.o > test.dump

# компилируем *.c, *.cpp сразу в объектный модуль
main.o: main.cpp mod_foo.h
	avr-gcc $(CFLAGS) -O2 -c main.cpp -o main.o
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d main.o > main.dump

# компиляция кода ассемблера
mod_foo.o: mod_foo.s
	avr-gcc -mmcu=$(MCU) -O2 -c mod_foo.s -o mod_foo.o
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d mod_foo.o > mod_foo.dump

# при желании мы можем их C/C++ файлов получить код ассемблера пригодный
# для дальшейшей компиляции
mod_foo.s: mod_foo.cpp mod_foo.h
	avr-gcc $(CFLAGS) -S mod_foo.cpp

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
