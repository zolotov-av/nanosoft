/*
* Модуль работы с таймерами/счетчиками/ШИМ ATmega16a
*
* Таймеры в AVR очень гибкие имеют множество настроек, цель данного модуля
* упростить решение типовых задач связанных с таймерами, счетчикам и ШИМ.
* 
* Данный модуль состоит только из заголовочного файла и не имеет собственного
* обработчика прерываний.
* 
* В ATmega16a имеем 3 счетчика:
*   Counter0 - 8 бит, один PWM-канал
*   Counter1 - 16 бит, два PWM-канала
*   Counter2 - 8 бит, один PWM-канал, поддержка часового кварца
* 
* (c) Alex V. Zolotov <zolotov-alex@shamangrad.net>, 2014
*     Fill free to copy, to compile, to use, to redistribute etc on your own risk.
*/

#ifndef __IOM16A_TIMER_H_
#define __IOM16A_TIMER_H_

#define TIM0_CLOCK_DISABLE    0
#define TIM0_CLOCK_1          1
#define TIM0_CLOCK_8          2
#define TIM0_CLOCK_64         3
#define TIM0_CLOCK_256        4
#define TIM0_CLOCK_1024       5
#define TIM0_CLOCK_EXT_FALL   6
#define TIM0_CLOCK_EXT_RISE   7

#define TIM0_FAST_PWM_NOPE      0
#define TIM0_FAST_PWM_RESERVED  1
#define TIM0_FAST_PWM_NORMAL    2
#define TIM0_FAST_PWM_INVERT    3

#define TIM0_FC_PWM_NOPE      0
#define TIM0_FC_PWM_RESERVED  1
#define TIM0_FC_PWM_NORMAL    2
#define TIM0_FC_PWM_INVERT    3

/**
* Прочитать значение счетчика
*/
inline char tim0_get_counter()
{
	return TCNT0;
}

/**
* Установить значение счетчика
*/
inline void tim0_set_counter(char value)
{
	TCNT0 = value;
}

/**
* Вернуть значение с которым сравнивается счетик
*/
inline char tim0_get_compare()
{
	return OCR0;
}

/**
* Установить значение сравнения счетчика
*/
inline void tim0_set_compare(char value)
{
	OCR0 = value;
}

/**
* Включить прерывание по совпадению считчика
*/
inline void tim0_enable_compare_interrupt()
{
	TIMSK |= (1 << OCIE0);
}

/**
* Выключить прерывание по совпадению счетчика
*/
inline void tim0_disable_compare_interrupt()
{
	TIMSK &= ~(1 << OCIE0);
}

/**
* Включить/выключить прерывание по совпадению счетчика
*/
inline void tim0_set_compare_interrupt(bool value)
{
	if ( value )
	{
		TIMSK |= (1 << OCIE0);
	}
	else
	{
		TIMSK &= ~(1 << OCIE0);
	}
}

/**
* Проверить включены ли прерывания по совпадению счетчика
*/
inline bool tim0_get_compare_interrupt()
{
	return TIMSK & (1 << OCIE0);
}

/**
* Включить прерывание по переполнению счетчика
*/
inline void tim0_enable_overflow_interrupt()
{
	TIMSK |= (1 << TOIE0);
}

/**
* Выключить прерывание по прерыванию счетчика
*/
inline void tim0_disable_overflow_interrupt()
{
	TIMSK &= ~(1 << TOIE0);
}

/**
* Включить/выключить прерывание по совпадению счетчика
*/
inline void tim0_set_overflow_interrupt(bool value)
{
	if ( value )
	{
		TIMSK |= (1 << TOIE0);
	}
	else
	{
		TIMSK &= ~(1 << TOIE0);
	}
}

/**
* Инициализация простого счетчика
* @param clock прескейлер счетчика
*/
inline void tim0_normal(char clock)
{
	TCCR0 = (0 << WGM01) | (0 << WGM00) | (clock & 0x07);
}

/**
* Инициализация CTC-счетчика (Clear Timer on Compare)
* @param clock прескейлер счетчика
*/
inline void tim0_ctc(char clock)
{
	TCCR0 = (1 << WGM01) | (0 << WGM00) | (clock & 0x07);
}

/**
* Инициализация Fast PWM
* @param clock прескейлер счетчика
* @param mode режим генерации сигнала
*/
inline void tim0_fast_pwm(char clock, char mode = TIM0_FAST_PWM_NORMAL)
{
	TCCR0 = (1 << WGM01) | (1 << WGM00) | ((mode & 0x03) << COM00) | (clock & 0x07);
	DDRB |= (1 << PB3);
}

/**
* Инициализация Phase Correct PWM
* @param clock прескейлер счетчика
* @param mode режим генерации сигнала
*/
inline void tim0_fc_pwm(char clock, char mode = TIM0_FC_PWM_NORMAL)
{
	TCCR0 = (0 << WGM01) | (1 << WGM00) | ((mode & 0x03) << COM00) | (clock & 0x07);
	DDRB |= (1 << PB3);
}

#endif // __IOM16A_TIMER_H_
