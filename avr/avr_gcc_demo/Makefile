
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

test.hex: test.o
	#
	# конвертируем объектный файл в *.hex
	avr-objcopy -O ihex test.o test.hex

test.o: main.o mod_foo.o
	#
	# компонуем несколько объектных модулей в один
	avr-gcc $(LDFLAGS) main.o mod_foo.o -o test.o
	#
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d test.o > test.dump

main.o: main.cpp mod_foo.h
	#
	# компилируем *.c, *.cpp сразу в объектный модуль
	avr-gcc $(CFLAGS) -O2 -c main.cpp -o main.o
	#
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d main.o > main.dump

mod_foo.o: mod_foo.s
	#
	# компиляция кода ассемблера
	avr-gcc -mmcu=$(MCU) -O2 -c mod_foo.s -o mod_foo.o
	#
	# при желании можем расковырять объектный модуль и посмотреть
	# что в него попало
	avr-objdump -d mod_foo.o > mod_foo.dump

mod_foo.s: mod_foo.cpp mod_foo.h
	#
	# при желании мы можем из C/C++ файлов получить код ассемблера,
	# пригодный для дальшейшей компиляции
	avr-gcc $(CFLAGS) -S mod_foo.cpp

clean:
	rm -rfv *.o *.s *.a *.dump *.hex
