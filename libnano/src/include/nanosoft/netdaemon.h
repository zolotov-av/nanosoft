#ifndef NANOSOFT_NETDAEMON_H
#define NANOSOFT_NETDAEMON_H

#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <queue>
#include <nanosoft/object.h>
#include <nanosoft/asyncobject.h>
#include <nanosoft/mutex.h>
#include <nanosoft/config.h>

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif // HAVE_GNUTLS

/**
* Callback таймера
*/
typedef void (*timer_callback_t) (const timeval &tv, void *data);

/**
* Главный класс сетевого демона
*/
class NetDaemon
{
private:
	/**
	* Файловый дескриптор epoll
	*/
	int epoll;
	
	/**
	* mutex
	*/
	nanosoft::Mutex mutex;
	
	/**
	* Активность демона
	* TRUE - активен, FALSE остановлен или в процессе останова
	*/
	bool active;
	
	struct timer
	{
		/**
		* Время когда надо активировать таймер
		*/
		int expires;
		
		/**
		* Callback таймера
		*/
		timer_callback_t callback;
		
		/**
		* Указатель на пользовательские данные
		*/
		void *data;
		
		/**
		* Конструктор по умолчанию
		*/
		timer() {
		}
		
		/**
		* Конструктор
		*/
		timer(int aExpires, timer_callback_t aCallback, void *aData):
			expires(aExpires), callback(aCallback), data(aData)
		{
		}
		
		/**
		* Оператор сравнения для приоритетной очереди
		*/
		bool operator < (const timer &t) const {
			return expires < t.expires;
		}
		
		/**
		* Запустить таймер
		*/
		void fire(const timeval &tv) {
			callback(tv, data);
		}
	};
	
	/**
	* Очередь таймеров
	*/
	typedef std::priority_queue<timer> timers_queue_t;
	
	/**
	* Таймеры
	*/
	timers_queue_t timers;
	
	/**
	 * Глобальный таймер
	 */
	timer_callback_t gtimer;
	
	/**
	 * Пользовательские данные для глобального таймера
	 */
	void *gtimer_data;
	
	/**
	* Максимальное число обслуживаемых объектов
	*/
	size_t limit;
	
	/**
	* Текущее число объектов
	*/
	size_t count;
	
	/**
	* Итератор объектов для корректного останова
	*/
	size_t iter;
	
	/**
	* Число потоков-воркеров
	*/
	int workerCount;
	
	/**
	* Число активных воркеров
	*/
	int activeCount;
	
	/**
	* Число таймеров в очереди
	*/
	int timerCount;
	
	/**
	* Структура описывающая один блок буфера
	*/
	struct block_t
	{
		/**
		* Ссылка на следующий блок
		*/
		block_t *next;
		
		/**
		* Данные блока
		*/
		char data[FDBUFFER_BLOCK_SIZE];
	};
	
	/**
	* Структура описывающая файловый дескриптор
	*/
	struct fd_info_t
	{
		/**
		* Мьютекс для сихронизации файлового буфера
		*/
		nanosoft::Mutex mutex;
		
		/**
		* Указатель на объект
		*/
		nanosoft::ptr<AsyncObject> obj;
		
		/**
		* Размер буферизованных данных (в байтах)
		*/
		size_t size;
		
		/**
		* Смещение в блоке к началу не записанных данных
		*/
		size_t offset;
		
		/**
		* Размер квоты для файлового дескриптора (в байтах)
		*/
		size_t quota;
		
		/**
		* Указатель на первый блок данных
		*/
		block_t *first;
		
		/**
		* Указатель на последний блок данных
		*/
		block_t *last;
	};
	
	/**
	* Размер буфера (в блоках)
	*/
	size_t buffer_size;
	
	/**
	* Число свободных блоков
	*/
	size_t free_blocks;
	
	/**
	* Буфер
	*/
	block_t *buffer;
	
	/**
	* Стек свободных блоков
	*/
	block_t *stack;
	
	/**
	* Таблица файловых дескрипторов
	*
	* хранит указатели на первый блок данных дескриптора или NULL
	* если нет буферизованных данных
	*/
	fd_info_t *fds;
	
	/**
	* Статус воркера
	*/
	enum worker_status_t {
		/**
		* Поток не запущен
		*/
		INACTIVE,
		
		/**
		* Поток в обычном рабочем цикле
		*/
		ACTIVE,
		
		/**
		* Подготовка к останову - уснуть и ждать останова других потоков
		*/
		SLEEP,
		
		/**
		* Останов - послать всем объектам событие onTerminate()
		*/
		TERMINATE
	};
	
	/**
	* Информация о воркере
	*/
	struct worker_t {
		/**
		* Ссылка на демона
		*/
		NetDaemon *daemon;
		
		/**
		* ID воркера
		*/
		int workerId;
		
#ifdef USE_PTHREAD
		/**
		* ID потока
		*/
		pthread_t thread;
		
		/**
		* Атрибуты потока
		*/
		pthread_attr_t attr;
#endif
		
		/**
		* Текущий статус воркера
		*/
		worker_status_t status;
		
		bool checked;
	};
	
	/**
	* Главный поток
	*/
	worker_t main;
	
	/**
	* Список воркеров
	*/
	worker_t *workers;
	
	/**
	* Действие активного цикла
	*/
	void doActiveAction(worker_t *worker);
	
	/**
	* Действие спящего цикла
	*/
	void doSleepAction(worker_t *worker);
	
	/**
	* Действие завещающего цикла
	*/
	void doTerminateAction(worker_t *worker);
	
	/**
	* Точка входа в воркер
	*/
	static void * workerEntry(void *pContext);
	
	/**
	* Возобновить работу с асинхронным объектом
	*/
	bool resetObject(nanosoft::ptr<AsyncObject> &object);
	
	/**
	* Размер стека воркера
	*/
	size_t workerStackSize;
	
	/**
	* Установить таймер
	* @param calltime время запуска таймера
	* @param callback функция обратного вызова
	* @param data указатель на пользовательские данные
	* @return TRUE - таймер установлен, FALSE - таймер установить не удалось
	*/
	bool callAt(time_t calltime, timer_callback_t callback, void *data);
	
	/**
	* Обработать таймеры
	*/
	void processTimers();
	
	/**
	* Выделить цепочку блоков достаточную для буферизации указаного размера
	* @param size требуемый размер в байтах
	* @return список блоков или NULL если невозможно выделить запрощенный размер
	*/
	block_t* allocBlocks(size_t size);
	
	/**
	* Освободить цепочку блоков
	* @param top цепочка блоков
	*/
	void freeBlocks(block_t *top);
	
	/**
	* Добавить данные в буфер (thread-unsafe)
	*
	* @param fd файловый дескриптор
	* @param fb указатель на описание файлового буфера
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE данные не приняты - нет места
	*/
	bool put(int fd, fd_info_t *fb, const char *data, size_t len);
protected:
	/**
	* Запустить воркеров
	*/
	void startWorkers();
	
	/**
	* Сменить статус воркеров
	*/
	void setWorkersState(worker_status_t status);
	
	/**
	* Остановить воркеров
	*/
	void stopWorkers();
	
	/**
	* Ожидать завершения работы всех воркеров
	*/
	void waitWorkers();
	
	/**
	* Удалить воркеров
	*/
	void freeWorkers();
	
	/**
	* Послать сигнал onTerminate() всем подконтрольным объектам
	*/
	void killObjects();
	
	/**
	* Обработка системной ошибки
	*/
	void stderror();
	
public:
	/**
	* Конструктор демона
	* @param fd_limit максимальное число одновременных виртуальных потоков
	* @param buf_size размер файлового буфера в блоках
	*/
	NetDaemon(int fd_limit, int buf_size);
	
	/**
	* Деструктор демона
	*/
	virtual ~NetDaemon();
	
	/**
	* Вернуть число воркеров
	*/
	int getWorkerCount();
	
	/**
	* Вернуть число подконтрольных объектов
	*/
	int getObjectCount();
	
	/**
	* Вернуть максимальное число подконтрольных объектов
	*/
	int getObjectLimit() { return limit; }
	
	/**
	* Вернуть размер буфера в блоках
	*/
	int getBufferSize() const { return buffer_size; }
	
	/**
	* Установить число воркеров
	*/
	void setWorkerCount(int count);
	
	/**
	* Вернуть размер стека воркера
	*/
	size_t getWorkerStackSize();
	
	/**
	* Установить размер стека воркера
	*/
	void setWorkerStackSize(size_t size);
	
	/**
	* Добавить асинхронный объект
	*/
	bool addObject(nanosoft::ptr<AsyncObject> object);
	
	/**
	* Уведомить NetDaemon, что объект изменил свою маску
	*/
	void modifyObject(nanosoft::ptr<AsyncObject> object);
	
	/**
	* Удалить асинхронный объект
	*/
	bool removeObject(nanosoft::ptr<AsyncObject> object);
	
	/**
	* Запустить демона
	*/
	int run();
	
	/**
	* Вернуть ID текущего воркера
	*/
	int wid();
	
	/**
	* Вернуть статус демона
	* TRUE - активен, FALSE отключен или в процессе останова
	*/
	bool getDaemonActive() { return active; }
	
	/**
	* Завершить работу демона
	*/
	void terminate();
	
	/**
	* Установить таймер однократного вызова
	* 
	* Указанная функция будет вызвана один раз в указанное время.
	* Можно устанавливать несколько таймеров и они будут вызваны в указанное
	* время
	* 
	* @param calltime время запуска таймера
	* @param callback функция обратного вызова
	* @param data указатель на пользовательские данные
	* @return TRUE - таймер установлен, FALSE - таймер установить не удалось
	*/
	template <class data_t>
	bool setTimer(time_t calltime, void (*callback)(const timeval &tv, data_t *data), data_t *data)
	{
		return callAt(calltime, reinterpret_cast<timer_callback_t>(callback), data);
	}
	
	/**
	* Установить глобальный таймер
	* 
	* Указанная функция будет вызываться перодически примерно каждые 200-300мс
	* Повторная установка таймера заменяет предыдущий, таким образом можно
	* указать только один глобальный таймер
	* 
	* @param calltime время запуска таймера
	* @param callback функция обратного вызова
	* @param data указатель на пользовательские данные
	* @return TRUE - таймер установлен, FALSE - таймер установить не удалось
	*/
	template <class data_t>
	bool setGlobalTimer(void (*callback)(const timeval &tv, data_t *data), data_t *data)
	{
		gtimer = reinterpret_cast<timer_callback_t>(callback);
		gtimer_data = data;
	}
	
	/**
	* Обработчик ошибок
	*
	* По умолчанию выводит все ошибки в stderr
	*/
	virtual void onError(const char *message);
	
	/**
	* Вернуть число свободных блоков в буфере
	* @return число свободных блоков в буфере
	*/
	size_t getFreeSize() { return free_blocks; }
	
	/**
	* Вернуть размер буферизованных данных
	* @param fd файловый дескриптор
	* @return размер буферизованных данных (в байтах)
	*/
	size_t getBufferedSize(int fd);
	
	/**
	* Вернуть квоту файлового дескриптора
	* @param fd файловый дескриптор
	* @return размер квоты (в блоках)
	*/
	size_t getQuota(int fd);
	
	/**
	* Установить квоту буфер файлового дескриптора
	* @param fd файловый дескриптор
	* @param quota размер квоты (в блоках)
	* @return TRUE квота установлена, FALSE квота не установлена
	*/
	bool setQuota(int fd, size_t quota);
	
	/**
	* Добавить данные в буфер (thread-safe)
	*
	* @param fd файловый дескриптор в который надо записать
	* @param data указатель на данные
	* @param len размер данных
	* @return TRUE данные приняты, FALSE данные не приняты - нет места
	*/
	bool put(int fd, const char *data, size_t len);
	
	/**
	* Записать данные из буфера в файл/сокет
	*
	* @param fd файловый дескриптор
	* @return TRUE буфер пуст, FALSE в буфере ещё есть данные
	*/
	bool push(int fd);
	
	/**
	* Удалить блоки файлового дескриптора
	*
	* @param fd файловый дескриптор
	*/
	void cleanup(int fd);
};

#endif // NANOSOFT_NETDAEMON_H
