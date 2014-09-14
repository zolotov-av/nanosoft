#include <stdio.h>
#include <stdint.h>

// Структура, описывающая заголовок WAV файла.
struct WAVHEADER
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
};

WAVHEADER hdr;

int main()
{
	printf("Hello world\n");
	FILE *f = fopen("test1.wav", "rb");
	if ( f == NULL )
	{
		fprintf(stderr, "open file fault\n");
		return 1;
	}
	
	int r = fread(&hdr, sizeof(hdr), 1, f);
	if ( r != 1 )
	{
		fprintf(stderr, "read fail: %d\n", r);
		fclose(f);
		return 0;
	}
	
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
	int i;
	float min, max = 0;
	
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
	
	fclose(f);
	return 0;
}
