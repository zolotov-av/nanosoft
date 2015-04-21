/*
* Модуль работы с АЦП (ADC) микроконтроллера ATmega16a
*
* (c) Alex V. Zolotov <zolotov-alex@shamangrad.net>, 2014
*     Fill free to copy, to compile, to use, to redistribute etc on your own risk.
*/

#ifndef __IOM16A_ADC_H_
#define __IOM16A_ADC_H_

/**
* Установить триггер для автоматического запуска АЦП
*/
inline void adc_set_trigger(char value)
{
	SFIOR = (SFIOR & 0xF0) | ((value & 0x07) << ADTS0);
}

#endif // __IOM16A_ADC_H_
