
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <nanosoft/netdaemon.h>
#include <nanosoft/error.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

using namespace std;
using namespace nanosoft;

#define DEFAULT_WORKER_STACK_SIZE (sizeof(size_t) * 1024 * 1024)

void my_gnutls_log_func( int level, const char *message)
{
	printf("gnutls: level=%d %s", level, message);
}

/**
* Конструктор демона
* @param fd_limit максимальное число одновременных виртуальных потоков
* @param buf_size размер файлового буфера в блоках
*/
NetDaemon::NetDaemon(int fd_limit, int buf_size):
	workerStackSize(DEFAULT_WORKER_STACK_SIZE),
	workerCount(0),
	activeCount(0),
	timerCount(0),
	gtimer(0),
	count(0),
	active(0)
{
	limit = fd_limit;
	epoll = epoll_create(fd_limit);
	
	fds = new fd_info_t[fd_limit];
	fd_info_t *fb, *fd_end = fds + fd_limit;
	for(fb = fds; fb < fd_end; fb++)
	{
		fb->obj = 0;
		fb->size = 0;
		fb->offset = 0;
		fb->quota = 0;
		fb->first = 0;
		fb->last = 0;
	}
	
	free_blocks = buffer_size = buf_size;
	buffer = new block_t[buf_size];
	stack = 0;
	block_t *block, *block_end = buffer + buf_size;
	for(block = buffer; block < block_end; block++)
	{
		memset(block->data, 0, sizeof(block->data));
		block->next = stack;
		stack = block;
	}
	
#ifdef HAVE_GNUTLS
	printf("BEFORE gnutls_global_init()\n");
	if ( gnutls_global_init() )
	{
		fprintf(stderr, "GnuTLS global init fault\n");
	}
	printf("AFTER gnutls_global_init()\n");
	
#ifdef DEBUG_TLS
	gnutls_global_set_log_level(9);
	gnutls_global_set_log_function(my_gnutls_log_func);
#endif // DEBUG_TLS
#endif // HAVE_GNUTLS
}

/**
* Деструктор демона
*/
NetDaemon::~NetDaemon()
{
	int r = ::close(epoll);
	if ( r < 0 ) stderror();
	
	delete [] fds;
	delete [] buffer;
	
#ifdef HAVE_GNUTLS
	gnutls_global_deinit();
#endif // HAVE_GNUTLS
}

/**
* Обработчик ошибок
*
* По умолчанию выводит все ошибки в stderr
*/
void NetDaemon::onError(const char *message)
{
	cerr << "[NetDaemon]: " << message << endl;
}

/**
* Обработка системной ошибки
*/
void NetDaemon::stderror()
{
	onError(strerror(errno));
}

/**
* Вернуть число воркеров
*/
int NetDaemon::getWorkerCount()
{
	return workerCount;
}

/**
* Установить число воркеров
*/
void NetDaemon::setWorkerCount(int count)
{
#ifdef USE_PTHREAD
	workerCount = count;
#endif
}

/**
* Вернуть число подконтрольных объектов
*/
int NetDaemon::getObjectCount()
{
	return count;
}

/**
* Вернуть размер стека воркера
*/
size_t NetDaemon::getWorkerStackSize()
{
	return workerStackSize;
}

/**
* Установить размер стека воркера
*/
void NetDaemon::setWorkerStackSize(size_t size)
{
	workerStackSize = size;
}

/**
* Добавить асинхронный объект
*/
bool NetDaemon::addObject(nanosoft::ptr<AsyncObject> object)
{
	printf("NetDaemon::addObject(%d)\n", object->fd);
	int r = false;
	struct epoll_event event;
	mutex.lock();
		fd_info_t *fb = &fds[object->fd];
		if ( fb->obj == 0 )
		{
			printf("NetDaemon::addObject(%d) real\n", object->fd);
			count ++;
			fb->obj = object;
			object->lock();
			object->daemon = this;
			
			// принудительно выставить O_NONBLOCK
			int flags = fcntl(object->fd, F_GETFL, 0);
			if ( flags >= 0 )
			{
				fcntl(object->fd, F_SETFL, flags | O_NONBLOCK);
			}
			
			event.events = object->getEventsMask();
			if ( fb->size > 0 ) event.events |= EPOLLOUT;
			event.data.fd = object->fd;
			r = epoll_ctl(epoll, EPOLL_CTL_ADD, object->fd, &event) == 0;
			/*if ( ! r  )
			{
				stderror();
				count --;
				objects[object->fd] = 0;
			}*/
		}
	mutex.unlock();
	printf("NetDaemon::addObject(%d) leave\n", object->fd);
	return r;
}

/**
* Уведомить NetDaemon, что объект изменил свою маску
*/
void NetDaemon::modifyObject(nanosoft::ptr<AsyncObject> object)
{
	printf("NetDaemon::modifyObject(%d)\n", object->fd);
	mutex.lock();
		if ( fds[object->fd].obj == object )
		{
			resetObject(object);
		}
	mutex.unlock();
	printf("NetDaemon::modifyObject(%d) leave\n", object->fd);
}

/**
* Удалить асинхронный объект
*/
bool NetDaemon::removeObject(nanosoft::ptr<AsyncObject> object)
{
	printf("NetDaemon::removeObject(%d)\n", object->fd);
	mutex.lock();
		if ( fds[object->fd].obj == object )
		{
			printf("NetDaemon::removeObject(%d) real\n", object->fd);
			if ( epoll_ctl(epoll, EPOLL_CTL_DEL, object->fd, 0) != 0 )
			{
				printf("NetDaemon::removeObject(%d) epoll_ctl != 0\n", object->fd);
				stderror();
			}
			fds[object->fd].obj = 0;
			object->daemon = 0;
			object->release();
			count --;
			if ( count == 0 ) stopWorkers();
		}
	mutex.unlock();
	printf("NetDaemon::removeObject(%d) leave\n", object->fd);
}

/**
* Возобновить работу с асинхронным объектом
*/
bool NetDaemon::resetObject(nanosoft::ptr<AsyncObject> &object)
{
	printf("NetDaemon::resetObject(%d)\n", object->fd);
	struct epoll_event event;
	event.events = object->getEventsMask();
	if ( fds[object->fd].size > 0 ) event.events |= EPOLLOUT;
	event.data.fd = object->fd;
	if ( epoll_ctl(epoll, EPOLL_CTL_MOD, object->fd, &event) != 0 ) stderror();
	printf("NetDaemon::resetObject(%d) leave\n", object->fd);
}

/**
* Действие активного цикла
*/
void NetDaemon::doActiveAction(worker_t *worker)
{
	ptr<AsyncObject> obj;
	struct epoll_event event;
	int r = epoll_wait(epoll, &event, 1, 200);
	if ( r > 0 )
	{
		mutex.lock();
			obj = fds[event.data.fd].obj;
		mutex.unlock();
		printf("NetDaemon::doActiveAction(fd=%d, obj=%p)\n", event.data.fd, obj.p);
		if ( obj != 0 )
		{
			obj->onEvent(event.events);
			mutex.lock();
				if ( fds[event.data.fd].obj == obj )
				{
					resetObject(obj);
				}
			mutex.unlock();
			obj = 0;
		}
	}
	if ( r < 0 ) fprintf(stderr, "#%d: %s\n", worker->workerId, nanosoft::stderror());
	if ( r == 0 ) processTimers();
}

/**
* Действие спящего цикла
*/
void NetDaemon::doSleepAction(worker_t *worker)
{
	if ( ! worker->checked )
	{
		worker->checked = true;
		mutex.lock();
			activeCount --;
			if ( activeCount == 0 )
			{
				iter = 0;
				setWorkersState(TERMINATE);
			}
		mutex.unlock();
		return;
	}
	
	printf("#%d: doSleepAction\n", worker->workerId);
	
	// просто засыпаем на как можно больший срок
	// лишь сигнал нас должен разбудить
	struct timespec tm;
	tm.tv_sec = 999999999;
	tm.tv_nsec = 0;
	nanosleep(&tm, 0);
}

/**
* Действие завещающего цикла
*/
void NetDaemon::doTerminateAction(worker_t *worker)
{
	if ( ! worker->checked )
	{
		worker->checked = true;
		mutex.lock();
			activeCount ++;
		mutex.unlock();
		return;
	}
	//printf("#%d: doTerminateAction\n", worker->workerId);
	
	ptr<AsyncObject> obj;
	mutex.lock();
		if ( iter < limit )
		{
			obj = fds[iter].obj;
			++ iter;
		}
		else
		{
			setWorkersState(ACTIVE);
			obj = 0;
		}
	mutex.unlock();
	if ( obj != 0 )
	{
		obj->terminate();
	}
}

/**
* Точка входа в воркер
*/
void* NetDaemon::workerEntry(void *pWorker)
{
	worker_t *worker = static_cast<worker_t *>(pWorker);
	NetDaemon *daemon = worker->daemon;
	
	printf("#%d: worker started\n", worker->workerId);
	
	while ( daemon->count > 0 && worker->status != INACTIVE )
	{
		switch ( worker->status )
		{
		case ACTIVE:
			daemon->doActiveAction(worker);
			break;
		case SLEEP:
			daemon->doSleepAction(worker);
			break;
		case TERMINATE:
			daemon->doTerminateAction(worker);
			break;
		}
	}
	
	printf("#%d: worker exited\n", worker->workerId);
	
	return 0;
}

/**
* Запустить воркеров
*/
void NetDaemon::startWorkers()
{
#ifdef USE_PTHREAD
	workers = new worker_t[workerCount];
	for(int i = 0; i < workerCount; i++)
	{
		workers[i].daemon = this;
		workers[i].workerId = i + 1;
		workers[i].status = ACTIVE;
		mutex.lock();
			activeCount++;
		mutex.unlock();
		pthread_attr_init(&workers[i].attr);
		pthread_attr_setstacksize(&workers[i].attr, getWorkerStackSize());
		pthread_attr_setdetachstate(&workers[i].attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&workers[i].thread, &workers[i].attr, workerEntry, &workers[i]);
	}
#endif
}

/**
* Сменить статус воркеров
*/
void NetDaemon::setWorkersState(worker_status_t status)
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		workers[i].status = status;
		workers[i].checked = false;
		pthread_kill(workers[i].thread, SIGHUP);
	}
#endif
	main.status = status;
	main.checked = false;
#ifdef USE_PTHREAD
	pthread_kill(main.thread, SIGHUP);
#endif
}

/**
* Остановить воркеров
*/
void NetDaemon::stopWorkers()
{
	cerr << "NetDaemon::stopWorkers()...\n";
	setWorkersState(INACTIVE);
}

/**
* Ожидать завершения работы всех воркеров
*/
void NetDaemon::waitWorkers()
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		void *status;
		int rc = pthread_join(workers[i].thread, &status);
		if ( rc ) fprintf(stderr, "[NetDaemon] pthread_join(#%) fault\n", i+1);
	}
#endif
}

/**
* Удалить воркеров
*/
void NetDaemon::freeWorkers()
{
#ifdef USE_PTHREAD
	for(int i = 0; i < workerCount; i++)
	{
		pthread_attr_destroy(&workers[i].attr);
	}
	delete [] workers;
#endif
}

/**
* Запустить демона
*/
int NetDaemon::run()
{
	active = 1;
	startWorkers();
	main.daemon = this;
	main.workerId = 0;
#ifdef USE_PTHREAD
	main.thread = pthread_self();
#endif
	main.status = ACTIVE;
	mutex.lock();
		activeCount++;
	mutex.unlock();
	workerEntry(&main);
	waitWorkers();
	freeWorkers();
	return 0;
}

/**
* Вернуть ID текущего воркера
*/
int NetDaemon::wid()
{
#ifdef USE_PTHREAD
	pthread_t tid = pthread_self();
	for(int i = 0; i < workerCount; i++)
	{
		if ( pthread_equal(workers[i].thread, tid) ) return i + 1;
	}
	if ( pthread_equal(main.thread, tid) ) return 0;
	return -1;
#endif
	return 0;
}

/**
* Завершить работу демона
*/
void NetDaemon::terminate()
{
	active = 0;
	onError("terminate...");
	//killObjects();
	setWorkersState(SLEEP);
}

/**
* Установить таймер
* @param calltime время запуска таймера
* @param callback функция обратного вызова
* @param data указатель на пользовательские данные
* @return TRUE - таймер установлен, FALSE - таймер установить не удалось
*/
bool NetDaemon::callAt(time_t calltime, timer_callback_t callback, void *data)
{
	printf("callAt(%d)\n", calltime);
	bool hup = false;
	mutex.lock();
		timers.push(timer(calltime, callback, data));
		hup = (timerCount == 0);
		timerCount++;
	mutex.unlock();
#ifdef USE_PTHREAD
	if ( hup )
	{
		for(int i = 0; i < workerCount; i++)
		{
			pthread_kill(workers[i].thread, SIGHUP);
		}
		pthread_kill(main.thread, SIGHUP);
	}
#endif
	return true;
}

/**
* Обработать таймеры
*/
void NetDaemon::processTimers()
{
	timer t;
	timeval tv;
	gettimeofday(&tv, 0);
	if ( gtimer )
	{
		gtimer(tv, gtimer_data);
	}
	while ( timerCount > 0 )
	{
		bool process = false;
		mutex.lock();
			t = timers.top();
			if ( t.expires <= tv.tv_sec )
			{
				timers.pop();
				timerCount --;
				process = true;
			}
		mutex.unlock();
		if ( process ) t.fire(tv);
		else break;
	}
}

/**
* Выделить цепочку блоков достаточную для буферизации указаного размера
* @param size требуемый размер в байтах
* @return список блоков или NULL если невозможно выделить запрощенный размер
*/
NetDaemon::block_t* NetDaemon::allocBlocks(size_t size)
{
	// размер в блоках
	size_t count = (size + FDBUFFER_BLOCK_SIZE - 1) / FDBUFFER_BLOCK_SIZE;
	
	if ( count == 0 ) return 0;
	
	if ( mutex.lock() )
	{
		block_t *block = 0;
		if ( count <= free_blocks )
		{
			block = stack;
			if ( count > 1 ) for(size_t i = 0; i < (count-1); i++) stack = stack->next;
			block_t *last = stack;
			stack = stack->next;
			last->next = 0;
			free_blocks -= count;
		}
		mutex.unlock();
		
		return block;
	}
	
	return 0;
}

/**
* Освободить цепочку блоков
* @param top цепочка блоков
*/
void NetDaemon::freeBlocks(block_t *top)
{
	if ( top == 0 ) return;
	block_t *last = top;
	size_t count = 1;
	while ( last->next ) { count++; last = last->next; }
	while ( 1 )
	{
		if ( mutex.lock() )
		{
			last->next = stack;
			stack = top;
			free_blocks += count;
			mutex.unlock();
			return;
		}
	}
}

/**
* Вернуть размер буферизованных данных
* @param fd файловый дескриптор
* @return размер буферизованных данных
*/
size_t NetDaemon::getBufferedSize(int fd)
{
	return ( fd >= 0 && fd < limit ) ? fds[fd].size : 0;
}

/**
* Вернуть квоту файлового дескриптора
* @param fd файловый дескриптор
* @return размер квоты
*/
size_t NetDaemon::getQuota(int fd)
{
	return ( fd >= 0 && fd < limit ) ? fds[fd].quota : 0;
}

/**
* Установить квоту буфер файлового дескриптора
* @param fd файловый дескриптор
* @param quota размер квоты
* @return TRUE квота установлена, FALSE квота не установлена
*/
bool NetDaemon::setQuota(int fd, size_t quota)
{
	if ( fd >= 0 && fd < limit )
	{
		fds[fd].quota = quota;
		return true;
	}
	return false;
}

/**
* Добавить данные в буфер (thread-unsafe)
*
* Если включена компрессия, то сначала сжать данные
*
* @param fd файловый дескриптор
* @param fb указатель на описание файлового буфера
* @param data указатель на данные
* @param len размер данных
* @return TRUE данные приняты, FALSE данные не приняты - нет места
*/
bool NetDaemon::put(int fd, fd_info_t *fb, const char *data, size_t len)
{
	if ( fb->quota != 0 && (fb->size + len) > fb->quota )
	{
		// превышение квоты
		return false;
	}
	
	block_t *block;
	
	if ( fb->size > 0 )
	{
		// смещение к свободной части последнего блока или 0, если последний
		// блок заполнен полностью
		size_t offset = (fb->offset + fb->size) % FDBUFFER_BLOCK_SIZE;
		
		// размер свободной части последнего блока
		size_t rest = offset > 0 ? FDBUFFER_BLOCK_SIZE - offset : 0;
		
		// размер недостающей части, которую надо выделить из общего буфера
		size_t need = len - rest;
		
		if ( len <= rest ) rest = len;
		else
		{
			// выделить недостающие блоки
			block = allocBlocks(need);
			if ( block == 0 ) return false;
		}
		
		// если последний блок заполнен не полностью, то дописать в него
		if ( offset > 0 )
		{
			memcpy(fb->last->data + offset, data, rest);
			fb->size += rest;
			data += rest;
			len -= rest;
			if ( len == 0 ) return true;
		}
		
		fb->last->next = block;
		fb->last = block;
	}
	else // fb->size == 0
	{
		block = allocBlocks(len);
		if ( block == 0 )
		{
			// нет буфера
			return false;
		}
		
		fb->first = block;
		fb->offset = 0;
	}
	
	// записываем полные блоки
	while ( len >= FDBUFFER_BLOCK_SIZE )
	{
		memcpy(block->data, data, FDBUFFER_BLOCK_SIZE);
		data += FDBUFFER_BLOCK_SIZE;
		len -= FDBUFFER_BLOCK_SIZE;
		fb->size += FDBUFFER_BLOCK_SIZE;
		fb->last = block;
		block = block->next;
	}
	
	// если что-то осталось записываем частичный блок
	if ( len > 0 )
	{
		memcpy(block->data, data, len);
		fb->size += len;
		fb->last = block;
	}
	
	return true;
}

/**
* Добавить данные в буфер (thread-safe)
*
* @param fd файловый дескриптор в который надо записать
* @param data указатель на данные
* @param len размер данных
* @return TRUE данные приняты, FALSE данные не приняты - нет места
*/
bool NetDaemon::put(int fd, const char *data, size_t len)
{
	// проверяем корректность файлового дескриптора
	if ( fd < 0 || fd >= limit )
	{
		// плохой дескриптор
		fprintf(stderr, "StanzaBuffer[%d]: wrong descriptor\n", fd);
		return false;
	}
	
	// проверяем размер, зачем делать лишние движения если len = 0?
	if ( len == 0 ) return true;
	
	// находим описание файлового буфера
	fd_info_t *fb = &fds[fd];
	
	if ( fb->mutex.lock() )
	{
		bool r = put(fd, fb, data, len);
		fb->mutex.unlock();
		return r;
	}
	
	return false;
}

/**
* Записать данные из буфера в файл/сокет
*
* @param fd файловый дескриптор
* @return TRUE буфер пуст, FALSE в буфере ещё есть данные
*/
bool NetDaemon::push(int fd)
{
	// проверяем корректность файлового дескриптора
	if ( fd < 0 || fd >= limit )
	{
		// плохой дескриптор
		fprintf(stderr, "StanzaBuffer[%d]: wrong descriptor\n", fd);
		return false;
	}
	
	// находим описание файлового буфера
	fd_info_t *fb = &fds[fd];
	
	if ( fb->mutex.lock() )
	{
		// список освободившихся блоков
		block_t *unused = 0;
		
		while ( fb->size > 0 )
		{
			// размер не записанной части блока
			size_t rest = FDBUFFER_BLOCK_SIZE - fb->offset;
			if ( rest > fb->size ) rest = fb->size;
			
			// попробовать записать
			ssize_t r = write(fd, fb->first->data + fb->offset, rest);
			if ( r <= 0 ) break;
			
			fb->size -= r;
			fb->offset += r;
			
			// если блок записан полностью,
			if ( r == rest )
			{
				// добавить его в список освободившихся
				block_t *block = fb->first;
				fb->first = block->next;
				fb->offset = 0;
				block->next = unused;
				unused = block;
			}
			else
			{
				// иначе пора прерваться и вернуться в epoll
				break;
			}
		}
		fb->mutex.unlock();
		freeBlocks(unused);
	}
	return fb->size <= 0;
}

/**
* Удалить блоки файлового дескриптора
*
* @param fd файловый дескриптор
*/
void NetDaemon::cleanup(int fd)
{
	// проверяем корректность файлового дескриптора
	if ( fd < 0 || fd >= limit )
	{
		// плохой дескриптор
		fprintf(stderr, "StanzaBuffer[%d]: wrong descriptor\n", fd);
		return;
	}
	
	fd_info_t *p = &fds[fd];
	freeBlocks(p->first);
	p->size = 0;
	p->offset = 0;
	p->quota = 0;
	p->first = 0;
	p->last = 0;
}
