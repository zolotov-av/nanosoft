//
// Простой программатор микроконтроллеров AVR для Raspberry Pi
// для программирования используются стандартные выводы шины GPIO
// (MOSI - 19, MISO - 21, SCK - 23) + 15й пин для подачи сигнала RESET
//
// Written on Raspberry Pi for Raspberry Pi
//
// (c) Alex V. Zolotov <zolotov-alex@shamangrad.net>, 2013
//    Fill free to copy, to compile, to use, to redistribute and etc on your own risk.
//

#include <bcm2835.h>
#include <stdio.h>
#include <time.h>

enum {
	AT_ACT_INFO,
	AT_ACT_CHECK,
	AT_ACT_WRITE,
	AT_ACT_ERASE
};

#define AT_PB3   RPI_V2_GPIO_P1_11
#define AT_PB4   RPI_V2_GPIO_P1_13
#define AT_RESET RPI_V2_GPIO_P1_15
#define AT_PWR   RPI_V2_GPIO_P1_17
#define AT_MOSI  RPI_V2_GPIO_P1_19
#define AT_MISO  RPI_V2_GPIO_P1_21
#define AT_SCK   RPI_V2_GPIO_P1_23
#define AT_GND   RPI_V2_GPIO_P1_25

#define GPIO_OUT(pin) bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP)
#define GPIO_INP(pin) bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT)

/**
* Действие которое надо выполнить
*/
int action;

/**
* Имя hex-файла
*/
const char *fname;

/**
* Нужно ли выводить дополнительный отладочный вывод или быть тихим?
*/
int verbose = 0;

/**
* Приостановить процесс на указанное число надосекунд
*/
void at_nanosleep(unsigned int ns)
{
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ns;
	nanosleep(&ts, 0);
}

/**
* Считать значение с пина
*/
int at_read(unsigned int pin)
{
	return bcm2835_gpio_lev(pin);
}

/**
* Выставить значение пина
*/
void at_write(unsigned int pin, unsigned int value)
{
	bcm2835_gpio_write(pin, value);
}

/**
* Инициализация пинов программатора
*/
void at_init()
{
	GPIO_INP(AT_PB3);
	GPIO_INP(AT_PB4);
	GPIO_OUT(AT_RESET);
	at_write(AT_RESET, HIGH);
//	GPIO_INP(AT_PWR);
	GPIO_OUT(AT_MOSI);
	at_write(AT_MOSI, LOW);
	GPIO_INP(AT_MISO);
	GPIO_OUT(AT_SCK);
	at_write(AT_SCK, LOW);
//	GPIO_INP(AT_GND);
}

/**
* Формирование тактов SPI
*/
void at_set_clk()
{
	at_write(AT_SCK, HIGH);
	at_nanosleep(1000);
}

/**
* Формирование тактов SPI
*/
void at_clr_clk()
{
	at_write(AT_SCK, LOW);
	at_nanosleep(1000);
}

/**
* Отправить байт по SPI и прочитать ответный
*/
unsigned char at_spi_io(unsigned char data)
{
	unsigned int result = 0;
	unsigned int bit;
	int i;
	for(i = 0; i < 8; i++)
	{
		bit = (data & 0x80) ? 1 : 0;
		at_write(AT_MOSI, bit ? HIGH : LOW);
		at_set_clk();
		data <<=1;

		bit = at_read(AT_MISO);
		result = result * 2 + bit;
		at_clr_clk();
	}
	return result;
}

/**
* Отправить комманду микроконтроллеру и прочтать ответ
* Все комманды размером 4 байта
*/
unsigned int at_io(unsigned int cmd)
{
	unsigned int result = 0;
	int i;
	for(i = 0; i < 4; i++)
	{
		unsigned char byte = cmd >> 24;
		byte = at_spi_io(byte);
		result = result * 256 + byte;
		cmd <<= 8;
	}
	return result;
}

/**
* Подать сигнал RESET, чтобы запустить устройство используй at_run()
*/
void at_reset()
{
	if ( verbose )
	{
		printf("reset device\n");
	}
	at_write(AT_RESET, HIGH);
}

/**
* Запустить устройство - вывести из состояния RESET
*/
void at_run()
{
	if ( verbose )
	{
		printf("run device\n");
	}
	at_write(AT_RESET, LOW);
}

/**
* Перезагрузить устройство
*/
void at_reboot()
{
	if ( verbose )
	{
		printf("reboot device\n");
	}
	at_write(AT_RESET, HIGH);
	bcm2835_delay(1);
	at_write(AT_RESET, LOW);
}

/**
* Подать сигнал RESET и командду "Programming Enable"
*/
int at_program_enable()
{
	at_write(AT_SCK, LOW);
	at_write(AT_MOSI, LOW);
	at_write(AT_RESET, LOW);
	bcm2835_delay(1);
	at_write(AT_RESET, HIGH);
	bcm2835_delay(1);
	at_write(AT_RESET, LOW);
	bcm2835_delay(1);
	
	unsigned int r = at_io(0xAC530000);
	int status = (r & 0xFF00) == 0x5300;
	if ( verbose )
	{
		const char *s = status ? "ok" : "fault";
		printf("at_program_enable(): %s\n", s);
	}
	return status;
}

/**
* Подать команду "Read Device Code"
*/
unsigned int at_chip_info()
{
	unsigned int sig = 0;
	sig = sig * 256 + (at_io(0x30000000) & 0xFF);
	sig = sig * 256 + (at_io(0x30000100) & 0xFF);
	sig = sig * 256 + (at_io(0x30000200) & 0xFF);
	if ( verbose )
	{
		printf("at_chip_info(): %08X\n", sig);
	}
	return sig;
}

/**
* Прочитать байт прошивки из устройства
*/
unsigned char at_read_memory(unsigned int addr)
{
	unsigned int cmd = (addr & 1) ? 0x28 : 0x20;
	unsigned int offset = (addr >> 1) & 0xFFFF;
	unsigned int byte = at_io( (cmd << 24) | (offset << 8 ) ) & 0xFF;
	return byte;
}

static active_page = 0;

/**
* Записать байт прошивки в устройство
* NOTE: байт сначала записывается в специальный буфер, фиксация
* данных происходит в функции at_flush(). Функция at_write_memory()
* сама переодически вызывает at_flush() поэтому нет необходимости
* часто вызывать at_flush() и необходимо только в конце, чтобы
* убедиться что последние данные будут записаны в устройство
*/
int at_write_memory(unsigned int addr, unsigned char byte)
{
	unsigned int cmd = (addr & 1) ? 0x48 : 0x40;
	unsigned int offset = (addr >> 1) & 0xFFFF;
	unsigned int page = (addr >> 1) & 0xFFF0;
	unsigned int x = 0;
	if ( page != active_page )
	{
		at_flush();
		active_page = page;
	}
	unsigned int result = at_io(x = (cmd << 24) | (offset << 8 ) | (byte & 0xFF) );
	unsigned int r = (result >> 16) & 0xFF;
	int status = r == cmd;
	if ( verbose )
	{
		printf(".");
		fflush(stdout);
		//printf("[%04X]=%02X%s ", offset, byte, (status ? "+" : "-"));
	}
	return status;
}

/**
* Завершить запись данных
*/
int at_flush()
{
	unsigned int cmd = 0x4C;
	unsigned int offset = active_page & 0xFFF0;
	unsigned int x = 0;
	unsigned int result = at_io(x = (cmd << 24) | (offset << 8 ) );
	unsigned int r = (result >> 16) & 0xFF;
	int status = r == cmd;
	if ( verbose )
	{
		printf("FLUSH[%04X]%s\n", offset, (status ? "+" : "-"));
	}
	bcm2835_delay(10);
	return status;
}

/**
* выдать сообщение об ошибке в hex-файле
*/
void at_wrong_file()
{
	printf("wrong hex-file\n");
}

/**
* Конверировать шестнадцатеричную цифру в число
*/
unsigned char at_hex_digit(char ch)
{
	if ( ch >= '0' && ch <= '9' ) return ch - '0';
	if ( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	if ( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	// TODO somethink...
	return 0;
}

/**
* Прочитать байт
*/
unsigned char at_hex_get_byte(const char *line, int i)
{
	int offset = i * 2 + 1;
	// TODO index limit checks
	return at_hex_digit(line[offset]) * 16 + at_hex_digit(line[offset+1]);
}

/**
* Прочитать слово (два байта)
*/
unsigned int at_hex_get_word(const char *line, int i)
{
	return at_hex_get_byte(line, i) * 256 + at_hex_get_byte(line, i+1);
}

/**
* Прочитать байт данных (читает из секции данных)
*/
unsigned char at_hex_get_data(const char *line, int i)
{
	return at_hex_get_byte(line, i + 4);
}

/**
* Сверить прошивку с данными из файла
*/
int at_check_firmware(const char *fname)
{
	FILE *f = fopen(fname, "r");
	if ( f )
	{
		int lineno = 0;
		int result = 1;
		while ( 1 )
		{
			char line[1024];
			const char *s = fgets(line, sizeof(line), f);
			if ( s == NULL ) break;
			//printf("%s", line);
			lineno++;
			if ( line[0] != ':' )
			{
				at_wrong_file();
				break;
			}
			unsigned char len = at_hex_get_byte(line, 0);
			unsigned int addr = at_hex_get_word(line, 1);
			unsigned char type = at_hex_get_byte(line, 3);
			unsigned char cc = at_hex_get_byte(line, 4 + len);
			//printf("len: %u, addr: %u, type: %u, cc: %u\n", len, addr, type, cc);
			if ( type == 0 )
			{
				int i;
				for(i = 0; i < len; i++)
				{
					unsigned char fbyte = at_hex_get_data(line, i);
					unsigned char dbyte = at_read_memory(addr + i);
					int r = (fbyte == dbyte);
					result = result && r;
					if ( verbose )
					{
						printf("%02X%s ", fbyte, (r ? "+" : "-"));
						fflush(stdout);
					}
				}
				if ( verbose ) printf("\n");
			}
			if ( type == 1 )
			{
				if ( verbose) printf("end of hex-file\n");
				break;
			}
		}
		fclose(f);
		if ( verbose )
		{
			char *st = result ? "same" : "differ";
			printf("firmware has checked: %s\n", st);
		}
		return result;
	}
	return 0;
}

/**
* Стереть чип
*/
int at_chip_erase()
{
	if ( verbose )
	{
		printf("erase device's firmware\n");
	}
	unsigned int r = at_io(0xAC800000);
	int ok = ((r >> 16) & 0xFF) == 0xAC;
	if ( ok )
	{
		bcm2835_delay(10);
		at_reboot();
		at_program_enable();
	}
	return ok;
}

/**
* Записать прошивку в устройство
*/
int at_write_firmware(const char *fname)
{
	FILE *f = fopen(fname, "r");
	if ( f )
	{
		int lineno = 0;
		int result = 1;
		while ( 1 )
		{
			char line[1024];
			const char *s = fgets(line, sizeof(line), f);
			if ( s == NULL ) break;
			//printf("%s", line);
			lineno++;
			if ( line[0] != ':' )
			{
				at_wrong_file();
				break;
			}
			unsigned char len = at_hex_get_byte(line, 0);
			unsigned int addr = at_hex_get_word(line, 1);
			unsigned char type = at_hex_get_byte(line, 3);
			unsigned char cc = at_hex_get_byte(line, 4 + len);
			//printf("len: %u, addr: %u, type: %u, cc: %u\n", len, addr, type, cc);
			if ( type == 0 )
			{
				int i;
				for(i = 0; i < len; i++)
				{
					unsigned char fbyte = at_hex_get_data(line, i);
					int r = at_write_memory(addr + i, fbyte);
					result = result && r;
					//printf("%02X%s ", fbyte, (r ? "+" : "-"));
				}
				//printf("\n");
			}
			if ( type == 1 )
			{
				if ( verbose ) printf("\nend of hex-file\n");
				break;
			}
		}
		at_flush();
		fclose(f);
		if ( verbose )
		{
			char *st = result ? "ok" : "fail";
			printf("memory write: %s\n", st);
		}
		return result;
	}
	return 0;
}

/**
* Действие - вывести информацию об устройстве
*/
int at_act_info()
{
	unsigned int info = at_chip_info();
	printf("chip signature: 0x%02X, 0x%02X, 0x%02X\n", (info >> 16) & 0xFF, (info >> 8) & 0xFF, info & 0xFF);
	return 1;
}

/**
* Действие - сверить прошивку в устрействе с файлом
*/
int at_act_check()
{
	int r = at_check_firmware(fname);
	if ( r ) printf("firmware is same\n");
	else printf("firmware differ\n");
	return 0;
}

/**
* Действие - записать прошивку в устройство
*/
int at_act_write()
{
	int r = at_write_firmware(fname);
	printf("firmware write: %s\n", (r ? "ok" : "fail"));
	return 0;
}

/**
* Действие - стереть чип
*/
int at_act_erase()
{
	int r = at_chip_erase();
	printf("chip erase: %s\n", (r ? "ok" : "fail"));
}

int run()
{
	switch ( action )
	{
	case AT_ACT_INFO: return at_act_info();
	case AT_ACT_CHECK: return at_act_check();
	case AT_ACT_WRITE: return at_act_write();
	case AT_ACT_ERASE: return at_act_erase();
	}
	printf("Victory!\n");
	return 0;
}

int help()
{
	printf("pigro :action: :filename: :verbose|quiet:\n");
	printf("  action:\n");
	printf("    info  - read chip info\n");
	printf("    check - read file and compare with device\n");
	printf("    write - read file and write to device\n");
	printf("    erase - just erase chip\n");
	return 0;
}

int main(int argc, char *argv[])
{
	int status = 0;
	
	if ( argc <= 1 ) return help();
	
	if ( strcmp(argv[1], "info") == 0 ) action = AT_ACT_INFO;
	else if ( strcmp(argv[1], "check") == 0 ) action = AT_ACT_CHECK;
	else if ( strcmp(argv[1], "write") == 0 ) action = AT_ACT_WRITE;
	else if ( strcmp(argv[1], "erase") == 0 ) action = AT_ACT_ERASE;
	else return help();
	
	fname = argc > 2 ? argv[2] : "firmware.hex";
	verbose = argc > 3 && strcmp(argv[3], "verbose") == 0;
	if ( verbose )
	{
		printf("fname: %s\n", fname);
	}
	
	if ( !bcm2835_init() )
	{
		printf("bcm2835_init() fault, try under root\n");
		return 1;
	}
	
	at_init();
	if ( at_program_enable() )
	{
		status = run();
		at_run();
	}
	else
	{
		fprintf(stderr, "ac_program_enable() failed\n");
	}
	
	GPIO_INP(AT_PB3);
	GPIO_INP(AT_PB4);
	GPIO_INP(AT_RESET);
//	GPIO_INP(AT_PWR);
	GPIO_INP(AT_MOSI);
	GPIO_INP(AT_MISO);
	GPIO_INP(AT_SCK);
//	GPIO_INP(AT_GND);
	
	if ( verbose )
	{
		printf("main() = %d\n", status);
	}
	return status;
}
