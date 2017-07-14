#ifndef FFC_SCALE_H
#define FFC_SCALE_H

#include <ffcairo/ffctypes.h>
#include <nanosoft/object.h>

/**
 * Класс-обертка для маштабирования и конвертации кадров
 *
 * Черновой вариант, хорошего и удобного решения пока не видно
 *
 * @note Для упрощения класс не скрывает внутренние структуры и не обеспечивает
 *   безопасность. Пользователь может свободно читать и использовать любые поля,
 *   но не должен пытаться перераспределить буфер или менять какие-либо из его
 *   параметров.
 */
class FFCScale: public Object
{
public:
	/**
	 * Контекст маштабирования и конвертации кадра
	 */
	SwsContext *ctx;
	
	/**
	 * Конструктор
	 */
	FFCScale();
	
	/**
	 * Деструктор
	 */
	~FFCScale();
	
	/**
	 * Инициализация маштабирования
	 *
	 * При необходимости сменить настройки маштабирования, init_scale()
	 * можно вывызывать без предварительного закрытия через close_scale()
	 */
	bool init_scale(AVFrame *dst, AVFrame *src);
	
	/**
	 * Маштабировать картику
	 */
	bool scale(AVFrame *dst, AVFrame *src);
	
	/**
	 * Финализация маштабирования
	 */
	void close_scale();
};

#endif // FFC_SCALE_H