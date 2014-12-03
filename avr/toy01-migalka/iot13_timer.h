#ifndef __TIMERS_H_
#define __TIMERS_H_

#define TIMER_WGM_NORMAL  0
#define TIMER_WGM_PWM     1
#define TIMER_WGM_CTC     2
#define TIMER_WGM_FPWM    3

#define TIMER_NORMAL_NOPE    0
#define TIMER_NORMAL_TOGGLE  1
#define TIMER_NORMAL_CLEAR   2
#define TIMER_NORMAL_SET     3

#define TIMER_PWM_NOPE       0
#define TIMER_PWM_RESERVED   1
#define TIMER_PWM_UP         2
#define TIMER_PWM_DOWN       3

#define TIMER_CTC_NOPE    0
#define TIMER_CTC_TOGGLE  1
#define TIMER_CTC_CLEAR   2
#define TIMER_CTC_SET     3

#define TIMER_FPWM_NOPE      0
#define TIMER_FPWM_RESERVED  1
#define TIMER_FPWM_NOINVERT  2
#define TIMER_FPWM_INVERT    3

#define TIMER_CLOCK_DISABLE    0
#define TIMER_CLOCK_1          1
#define TIMER_CLOCK_8          2
#define TIMER_CLOCK_64         3
#define TIMER_CLOCK_256        4
#define TIMER_CLOCK_1024       5
#define TIMER_CLOCK_EXT_FALL   6
#define TIMER_CLOCK_EXT_RISE   7

//#define TIM2_WGM1(mode) ((mode & 0x02) >> 1)
//#define TIM2_WGM0(mode) (mode & 0x01)
//#define TIM2_WGM(mode) ((TIM2_WGM0(mode)<<WGM20) | (TIM2_WGM1(mode)<<WGM21))

//#define TIM2_COM1(mode) ((mode & 0x02) >> 1)
//#define TIM2_COM0(mode) (mode & 0x01)
//#define TIM2_COM(mode) ((TIM2_COM0(mode)<<COM20) | (TIM2_COM1(mode)<<COM21))

//#define TIM2_CS2(mode) ((mode >> 2) & 0x01)
//#define TIM2_CS1(mode) ((mode >> 1) & 0x01)
//#define TIM2_CS0(mode) (mode & 0x01)
//#define TIM2_CS(mode) ((TIM2_CS2(mode)<<CS22) | (TIM2_CS1(mode)<<CS21) | (TIM2_CS0(mode)<<CS20))

//#define TIM2_TCCR(wgm, com, clock) (TIM2_WGM(wgm) | TIM2_COM(com) | TIM2_CS(clock))

#ifndef TIM0_OCI_ENABLE
#define TIM0_OCI_ENABLE 0
#endif

#ifndef TIM0_TOI_ENABLE
#define TIM0_TOI_ENABLE 0
#endif

//#define TIM2_DEFAULT_TIMSK (TIM0_OCI_ENABLE<<OCIE0)|(TIM0_TOI_ENABLE<<TOIE0)| \
	(TIM2_OCI_ENABLE<<OCIE2)|(TIM2_TOI_ENABLE<<TOIE2)


/**
* Вернуть значение счетчика Timer0
*/
inline unsigned char timer0_value()
{
	return TCNT0;
}

/**
* Вернуть значение с которым сравнивается таймер
*/
inline unsigned char timer0_get_compareA()
{
	return OCR0A;
}

/**
* Вернуть значение с которым сравнивается таймер
*/
inline unsigned char timer0_get_compareB()
{
	return OCR0B;
}

/**
* Установить значение с которым сравнивается таймер
*/
inline void timer0_set_compareA(unsigned char value)
{
	OCR0A = value;
}

/**
* Установить значение с которым сравнивается таймер
*/
inline void timer0_set_compareB(unsigned char value)
{
	OCR0B = value;
}

/**
* Конфигурировать Timter2
* @param wgm режим генерации формы сингнала
* @param com режим модуля сравнения
* @param clock режим таймера/прескейлера
*/
//inline void timer0_configure(char wgm, char com, char clock)
//{
//	TCCR0 = TIM2_TCCR(wgm, com, clock);
//}

/**
* Инициализировать Timter2
* @param wgm режим генерации формы сингнала
* @param com режим модуля сравнения
* @param clock режим таймера/прескейлера
*/
//inline void timer2_init(char wgm, char com, char clock)
//{
//	timer2_configure(wgm, com, clock);
//	TIMSK = TIM2_DEFAULT_TIMSK;
//}

#endif // __TIMERS_H_
