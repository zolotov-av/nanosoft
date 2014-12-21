//
// Программатор микроконтроллеров AVR.
// 
// Изначально был написан на Raspberry Pi и для Raspberry Pi и использовал
// шину GPIO на Raspberry Pi, к которой непосредственно подключался
// программируемый контроллер. Эта версия программатора использует внешений
// самодельный переходник USB <-> SPI и может запускаться теоретически на
// любой linux-системе.
//
// (c) Alex V. Zolotov <zolotov-alex@shamangrad.net>, 2013
//    Fill free to copy, to compile, to use, to redistribute etc on your own risk.
//

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>  /* Объявления стандартных функций UNIX */
#include <fcntl.h>   /* Объявления управления файлами */
#include <errno.h>   /* Объявления кодов ошибок */
#include <termios.h> /* Объявления управления POSIX-терминалом */

/**
* Файловый дескриптор (виртуального) последовательного порта
*/
int serial_fd = 0;

/**
* Инициализация последовательного порта
*/
int serial_init()
{
	serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (serial_fd == -1)
	{
		perror("serial_init() Unable to open /dev/ttyUSB0");
		return 0;
	}
	
	struct termios options;

	// Получение текущих опций для порта...
	tcgetattr(serial_fd, &options);
	
	speed_t speed = B19200;
	
	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	
	options.c_cflag &= ~CRTSCTS;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
	options.c_iflag = 0;
	//options.c_iflag &= ~(IXON | IXOFF | IXANY);
	//options.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	
	options.c_oflag = 0;
	//options.c_oflag &= ~OPOST;
	//options.c_oflag &= ~(ICRNL | INLCR | IGNCR);
	
	// Установка новых опций для порта...
	tcsetattr(serial_fd, TCSANOW, &options);
	
	fcntl(serial_fd, F_SETFL, 0);
	
	return 1;
}

// Структура, описывающая заголовок WAV файла.
typedef struct _WAVHEADER
{
	// WAV-формат начинается с RIFF-заголовка:
	
	// Содержит символы "RIFF" в ASCII кодировке
	// (0x52494646 в big-endian представлении)
	char chunkId[4];
	
	// 36 + subchunk2Size, или более точно:
	// 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
	// Это оставшийся размер цепочки, начиная с этой позиции.
	// Иначе говоря, это размер файла - 8, то есть,
	// исключены поля chunkId и chunkSize.
	uint32_t chunkSize;
	
	// Содержит символы "WAVE"
	// (0x57415645 в big-endian представлении)
	char format[4];
	
	// Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
	// Подцепочка "fmt " описывает формат звуковых данных:
	// Содержит символы "fmt "
	// (0x666d7420 в big-endian представлении)
	char subchunk1Id[4];
	
	// 16 для формата PCM.
	// Это оставшийся размер подцепочки, начиная с этой позиции.
	uint32_t subchunk1Size;
	
	// Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
	// Для PCM = 1 (то есть, Линейное квантование).
	// Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
	uint16_t audioFormat;
	
	// Количество каналов. Моно = 1, Стерео = 2 и т.д.
	uint16_t numChannels;
	
	// Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
	uint32_t sampleRate;
	
	// sampleRate * numChannels * bitsPerSample/8
	uint32_t byteRate;
	
	// numChannels * bitsPerSample/8
	// Количество байт для одного сэмпла, включая все каналы.
	uint16_t blockAlign;
	
	// Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
	uint16_t bitsPerSample;
	
	// Подцепочка "data" содержит аудио-данные и их размер.
	
	// Содержит символы "data"
	// (0x64617461 в big-endian представлении)
	char subchunk2Id[4];
	
	// numSamples * numChannels * bitsPerSample/8
	// Количество байт в области данных.
	uint32_t subchunk2Size;
	
	// Далее следуют непосредственно Wav данные.
} WAVHEADER;

WAVHEADER hdr;

int main(int argc, char *argv[])
{
	if ( ! serial_init() )
	{
		printf("serial init fault\n");
		return 1;
	}
	
	printf("Hello world\n");
	
	unsigned char byte;

	FILE *f = fopen("test1.wav", "wb");
	if ( f == NULL )
	{
		fprintf(stderr, "open file fault\n");
		return 1;
	}
	
	// Содержит символы "RIFF" в ASCII кодировке
	// (0x52494646 в big-endian представлении)
	memcpy(hdr.chunkId, "RIFF", 4);
	
	// Содержит символы "WAVE"
	// (0x57415645 в big-endian представлении)
	memcpy(hdr.format, "WAVE", 4);
	
	// Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
	// Подцепочка "fmt " описывает формат звуковых данных:
	// Содержит символы "fmt "
	// (0x666d7420 в big-endian представлении)
	memcpy(hdr.subchunk1Id, "fmt ", 4);
	
	// 16 для формата PCM.
	// Это оставшийся размер подцепочки, начиная с этой позиции.
	hdr.subchunk1Size = 16;
	
	// Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
	// Для PCM = 1 (то есть, Линейное квантование).
	// Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
	hdr.audioFormat = 1;
	
	// Количество каналов. Моно = 1, Стерео = 2 и т.д.
	hdr.numChannels = 1;
	
	// Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
	hdr.sampleRate = 8000;
	
	// sampleRate * numChannels * bitsPerSample/8
	hdr.byteRate = hdr.numChannels * hdr.sampleRate;
	
	// numChannels * bitsPerSample/8
	// Количество байт для одного сэмпла, включая все каналы.
	hdr.blockAlign = 1;
	
	// Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
	hdr.bitsPerSample = 8;
	
	// Подцепочка "data" содержит аудио-данные и их размер.
	
	// Содержит символы "data"
	// (0x64617461 в big-endian представлении)
	memcpy(hdr.subchunk2Id, "data", 4);
	
	// numSamples * numChannels * bitsPerSample/8
	// Количество байт в области данных.
	unsigned int countSamples = 8000 * 20;
	hdr.subchunk2Size = countSamples * hdr.numChannels;
	
	// 36 + subchunk2Size, или более точно:
	// 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
	// Это оставшийся размер цепочки, начиная с этой позиции.
	// Иначе говоря, это размер файла - 8, то есть,
	// исключены поля chunkId и chunkSize.
	hdr.chunkSize = 36 + hdr.subchunk2Size;
	
	printf("chunkId: %c%c%c%c\n", hdr.chunkId[0], hdr.chunkId[1], hdr.chunkId[2], hdr.chunkId[3]);
	printf("format: %c%c%c%c\n", hdr.format[0], hdr.format[1], hdr.format[2], hdr.format[3]);
	printf("audioFormat: %d\n", hdr.audioFormat);
	printf("numChannels: %d\n", hdr.numChannels);
	printf("sampleRate: %d\n", hdr.sampleRate);
	printf("byteRate: %d\n", hdr.byteRate);
	printf("blockAlign: %d\n", hdr.blockAlign);
	printf("bitsPerSample: %d\n", hdr.bitsPerSample);
	printf("subchunk2Id: %c%c%c%c\n", hdr.subchunk2Id[0], hdr.subchunk2Id[1], hdr.subchunk2Id[2], hdr.subchunk2Id[3]);
	printf("subchunk2Size: %d\n", hdr.subchunk2Size);
	unsigned int bytePerSample = hdr.numChannels * (hdr.bitsPerSample / 8);
	unsigned int numSamples = hdr.subchunk2Size / bytePerSample;
	printf("calc bytePerSample: %d\n", bytePerSample);
	printf("calc numSamples: %d\n", numSamples);
	
	int r = fwrite(&hdr, sizeof(hdr), 1, f);
	if ( r != 1 )
	{
		fprintf(stderr, "write fail: %d\n", r);
		fclose(f);
		return 0;
	}
	
	int i;
	for(i = 0; i < countSamples; i++)
	{
		int r = read(serial_fd, &byte, sizeof(byte));
		if ( r == sizeof(byte) )
		{
			printf("byte[%d/%d]: 0x%02X\n", i, countSamples, byte);
			byte += 128;
			fwrite(&byte, 1, 1, f);
		}
		else if ( r == -1 )
		{
			perror("read fault");
		}
	}
	
#if 0
	FILE *fo = fopen("out.wav", "wb");
	if ( fo == NULL )
	{
		fprintf(stderr, "fail open out.wav for write\n");
		return 1;
	}
	hdr.bitsPerSample = 16;
	hdr.byteRate = hdr.sampleRate * hdr.numChannels * (hdr.bitsPerSample / 8);
	hdr.blockAlign = hdr.numChannels * (hdr.bitsPerSample / 8);
	hdr.subchunk2Size = numSamples * hdr.numChannels * (hdr.bitsPerSample / 8);
	fwrite(&hdr, sizeof(hdr), 1, fo);
	
	for(i = 0; i < numSamples; i++)
	{
		unsigned char data[32];
		r = fread(data, bytePerSample, 1, f);
		if ( r != 1 )
		{
			fprintf(stderr, "read sample failed\n");
			break;
		}
		uint8_t lchi = data[0];
		uint8_t rchi = 0;//data[3] * 256 + data[2];
		
		float lch = lchi;
		float rch = rchi;
		
		//lch = (lch - 128);
		//rch = rch * 256;
		//printf("L: %f, R: %f\n", lch, rch);
		
		if ( lchi < min ) min = lchi;
		if ( lchi > max ) max = lchi;
		
		int16_t lcho = lch;
		//int16_t rcho = rch;
		
		//if ( lcho < min ) min = lcho;
		//if ( lcho > max ) max = lcho;
		
		//data[0] = (uint32_t(lcho) >> 24) & 0xFF;
		//data[1] = (uint32_t(lcho) >> 16) & 0xFF;
		data[0] = 0;//(uint8_t(lcho) >> 8) & 0xFF;
		data[1] = uint8_t(lcho) & 0xFF;
		
		//data[4] = (uint32_t(rcho) >> 24) & 0xFF;
		//data[5] = (uint32_t(rcho) >> 16) & 0xFF;
		//data[6] = (uint32_t(rcho) >> 8 ) & 0xFF;
		//data[1] = uint32_t(rcho) & 0xFF;
		
		fwrite(data, 2, 1, fo);
	}
	printf("min: %f, max: %f\n", min, max);
#endif
	fclose(f);
	
	
	return 0;
}
