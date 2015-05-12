/*
* Модуль работы с таймерами/счетчиками/ШИМ ATtiny2313
*
* Таймеры в AVR очень гибкие имеют множество настроек и нюансов что осложняет
* создание единого прозрачного API без раздувания кода. Цель данного модуля
* упростить решение типовых задач связанных с таймерами, счетчикам, ШИМ
* и не претендует на покрытие всех возможностей таймеров ATtiny2313.
*
* Данный модуль состоит только из заголовочного файла и не имеет собственного
* обработчика прерываний.
*
* В ATtiny2313 имеем 2 счетчика и 4 ШИМ-канала
*   Counter0 - 8 бит, два ШИМ-канала
*   Counter1 - 16 бит, два ШИМ-канала, захват счетчика
*
* (c) Alex V. Zolotov <zolotov-alex@shamangrad.net>, 2015
*     Fill free to copy, to compile, to use, to redistribute etc on your own risk.
*/

#ifndef __IOT2313_TIMER_H_
#define __IOT2313_TIMER_H_

#define TIM0_CLOCK_DISABLE    0
#define TIM0_CLOCK_1          1
#define TIM0_CLOCK_8          2
#define TIM0_CLOCK_64         3
#define TIM0_CLOCK_256        4
#define TIM0_CLOCK_1024       5
#define TIM0_CLOCK_EXT_FALL   6
#define TIM0_CLOCK_EXT_RISE   7

#define TIM1_CLOCK_DISABLE    0
#define TIM1_CLOCK_1          1
#define TIM1_CLOCK_8          2
#define TIM1_CLOCK_64         3
#define TIM1_CLOCK_256        4
#define TIM1_CLOCK_1024       5
#define TIM1_CLOCK_EXT_FALL   6
#define TIM1_CLOCK_EXT_RISE   7


typedef unsigned char tim0_t;
typedef unsigned int tim1_t;

/**
* Прочитать значение счетчика Counter0
*/
inline tim0_t tim0_get_counter()
{
	return TCNT0;
}

/**
* Установить значение счетчика Counter0
*/
inline void tim0_set_counter(tim0_t value)
{
	TCNT0 = value;
}

/**
* Вернуть A-значение с которым сравнивается счетик Counter0
*/
inline tim0_t tim0_get_compareA()
{
	return OCR0A;
}

/**
* Вернуть B-значение с которым сравнивается счетик Counter0
*/
inline tim0_t tim0_get_compareB()
{
	return OCR0B;
}

/**
* Установить A-значение сравнения счетчика Counter0
*/
inline void tim0_set_compareA(tim0_t value)
{
	OCR0A = value;
}

/**
* Установить A-значение сравнения счетчика Counter0
*/
inline void tim0_set_compareB(tim0_t value)
{
	OCR0B = value;
}

/**
* Включить прерывание по совпадению считчика Counter0 с A-значением
*/
inline void tim0_enable_compareA_interrupt()
{
	TIMSK |= (1 << OCIE0A);
}

/**
* Включить прерывание по совпадению считчика Counter0 с B-значением
*/
inline void tim0_enable_compareB_interrupt()
{
	TIMSK |= (1 << OCIE0B);
}

/**
* Выключить прерывание по совпадению счетчика Counter0 с A-значением
*/
inline void tim0_disable_compareA_interrupt()
{
	TIMSK &= ~(1 << OCIE0A);
}

/**
* Выключить прерывание по совпадению счетчика Counter0 с B-значением
*/
inline void tim0_disable_compareB_interrupt()
{
	TIMSK &= ~(1 << OCIE0B);
}

/**
* Включить/выключить прерывание по совпадению счетчика Counter0 с A-значением
*/
inline void tim0_set_compareA_interrupt(bool value)
{
	if ( value )
	{
		tim0_enable_compareA_interrupt();
	}
	else
	{
		tim0_disable_compareA_interrupt();
	}
}

/**
* Включить/выключить прерывание по совпадению счетчика Counter0 с B-значением
*/
inline void tim0_set_compareB_interrupt(bool value)
{
	if ( value )
	{
		tim0_enable_compareB_interrupt();
	}
	else
	{
		tim0_disable_compareB_interrupt();
	}
}

/**
* Проверить включены ли прерывания по совпадению счетчика Counter0 с A-значением
*/
inline bool tim0_get_compareA_interrupt()
{
	return TIMSK & (1 << OCIE0A);
}

/**
* Проверить включены ли прерывания по совпадению счетчика Counter0 с B-значением
*/
inline bool tim0_get_compareB_interrupt()
{
	return TIMSK & (1 << OCIE0B);
}

/**
* Включить прерывание по переполнению счетчика Counter0
*/
inline void tim0_enable_overflow_interrupt()
{
	TIMSK |= (1 << TOIE0);
}

/**
* Выключить прерывание по переполнению счетчика Counter0
*/
inline void tim0_disable_overflow_interrupt()
{
	TIMSK &= ~(1 << TOIE0);
}

/**
* Включить/выключить прерывание по переполнению счетчика Counter0
*/
inline void tim0_set_overflow_interrupt(bool value)
{
	if ( value )
	{
		tim0_enable_overflow_interrupt();
	}
	else
	{
		tim0_disable_overflow_interrupt();
	}
}

/**
* Инициализация таймер Counter0
* @param wgmode режим генерации формы (Waveform Generation mode)
* @param clock прескейлер счетчика
* @param modeA режим вывода канала A
* @param modeB режим вывода канала B
*/
inline void tim0_init(char wgmode, char clock, char modeA, char modeB)
{
	TCCR0A = (wgmode & 0x03) | ((modeA & 0x03) << COM0A0) | ((modeB & 0x03) << COM0B0);
	TCCR0B = (((wgmode >> 2) & 0x01) << WGM02) | (clock & 0x07);
}

/**
* Инициализация простого счетчика
* @param clock прескейлер счетчика
*/
inline void tim0_normal(char clock)
{
	tim0_init(0, clock, 0, 0);
}

/**
* Инициализация CTC-счетчика (Clear Timer on Compare)
* @param clock прескейлер счетчика
*/
inline void tim0_ctc(char clock)
{
	tim0_init(2, clock, 0, 0);
}

/**
* Прочитать значение счетчика Counter1
*/
inline tim1_t tim1_get_counter()
{
	return TCNT1;
}

/**
* Установить значение счетчика Counter1
*/
inline void tim1_set_counter(tim1_t value)
{
	TCNT1 = value;
}

/**
* Вернуть A-значение с которым сравнивается счетик Counter1
*/
inline tim1_t tim1_get_compareA()
{
	return OCR1A;
}

/**
* Вернуть B-значение с которым сравнивается счетик Counter1
*/
inline tim1_t tim1_get_compareB()
{
	return OCR1B;
}

/**
* Установить A-значение сравнения счетчика Counter1
*/
inline void tim1_set_compareA(tim1_t value)
{
	OCR1A = value;
}

/**
* Установить B-значение сравнения счетчика Counter1
*/
inline void tim1_set_compareB(tim1_t value)
{
	OCR1B = value;
}

/**
* Вернуть значение захваченного значения счетчика Counter1
*/
inline tim1_t tim1_get_capture()
{
	return ICR1;
}

/**
* Установить значение регистра ICR1
*
* используется для задания разрешения счетчика в некоторых PWM-режимах
*/
inline void tim1_set_capture(tim1_t value)
{
	ICR1 = value;
}

/**
* Включить прерывание по совпадению считчика Counter1 со значением A
*/
inline void tim1_enable_compareA_interrupt()
{
	TIMSK |= (1 << OCIE1A);
}

/**
* Включить прерывание по совпадению считчика Counter1 со значением B
*/
inline void tim1_enable_compareB_interrupt()
{
	TIMSK |= (1 << OCIE1B);
}

/**
* Выключить прерывание по совпадению счетчика Counter1 со значением A
*/
inline void tim1_disable_compareA_interrupt()
{
	TIMSK &= ~(1 << OCIE1A);
}

/**
* Выключить прерывание по совпадению счетчика Counter1 со значением B
*/
inline void tim1_disable_compareB_interrupt()
{
	TIMSK &= ~(1 << OCIE1B);
}

/**
* Включить/выключить прерывание по совпадению счеитчка Counter1 со значением A
*/
inline void tim1_set_compareA_interrupt(bool value)
{
	if ( value )
	{
		tim1_enable_compareA_interrupt();
	}
	else
	{
		tim1_disable_compareA_interrupt();
	}
}

/**
* Включить/выключить прерывание по совпадению счеитчка Counter1 со значением B
*/
inline void tim1_set_compareB_interrupt(bool value)
{
	if ( value )
	{
		tim1_enable_compareB_interrupt();
	}
	else
	{
		tim1_disable_compareB_interrupt();
	}
}

/**
* Включить прерывание по захвату счетчика Counter1
*/
inline void tim1_enable_capture_interrupt()
{
	TIMSK |= (1 << ICIE1);
}

/**
* Выключить прерывание по захвату счетчика Counter1
*/
inline void tim1_disable_capture_interrupt()
{
	TIMSK &= ~(1 << ICIE1);
}

/**
* Включить/выключить прерывание по захвату счетчика Counter1
*/
inline void tim1_set_capture_interrupt(bool value)
{
	if ( value )
	{
		tim1_enable_capture_interrupt();
	}
	else
	{
		tim1_disable_capture_interrupt();
	}
}

/**
* Включить прерывание по переполнению счетчика Counter1
*/
inline void tim1_enable_overflow_interrupt()
{
	TIMSK |= (1 << TOIE1);
}

/**
* Выключить прерывание по переполнению счетчика Counter1
*/
inline void tim1_disable_overflow_interrupt()
{
	TIMSK &= ~(1 << TOIE1);
}

/**
* Включить/выключить прерывание по переполнению счетчика Counter1
*/
inline void tim1_set_overflow_interrupt(bool value)
{
	if ( value )
	{
		tim1_enable_overflow_interrupt();
	}
	else
	{
		tim1_disable_overflow_interrupt();
	}
}

/**
* Инициализация таймер Counter1
* @param wgmode режим генерации формы (Waveform Generation mode)
* @param clock прескейлер счетчика
* @param modeA режим вывода канала A
* @param modeB режим вывода канала B
*/
inline void tim1_init(char wgmode, char clock, char modeA, char modeB)
{
	TCCR1A = (wgmode & 0x03) | ((modeA & 0x03) << COM1A0) | ((modeB & 0x03) << COM1B0);
	TCCR1B = (((wgmode >> 2) & 0x03) << WGM12) | (clock & 0x07);
}


/**
* Инициализация простого счетчика
* @param clock прескейлер счетчика
*/
inline void tim1_normal(char clock)
{
	tim1_init(0, clock, 0, 0);
}

#endif // __IOT2313_TIMER_H_
