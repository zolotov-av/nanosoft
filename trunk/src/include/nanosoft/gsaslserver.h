#ifndef NANOSOFT_GSASLSERVER_H
#define NANOSOFT_GSASLSERVER_H

#include <nanosoft/saslserver.h>
#include <gsasl.h>
#include <vector>
#include <string>

class GSASLSession;

/**
* Абстрактный класс серверной реализации SASL
*
* см. http://asg.web.cmu.edu/sasl/
*/
class GSASLServer: public SASLServer
{
private:
	/**
	* GNU SASL contetext
	*/
	Gsasl *ctx;
	
	/**
	* Проверка статуса выполенения вызово GNU SASL
	*/
	static void check(int status);
	
	/**
	* Callback для GNU SASL
	*/
	static int gsaslCallback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop);
	
public:
	/**
	* Конструктор
	*/
	GSASLServer();
	
	/**
	* Конструктор
	*/
	~GSASLServer();
	
	/**
	* Вернуть список методов авторизации
	*/
	mechanisms_t getMechanisms();
	
	/**
	* Начать авторизацию
	* @param service сервис
	* @param host хост
	* @param mech выбранный механизм авторизации
	*/
	GSASLSession* start(const std::string &service, const std::string &host, const std::string &mech);
	
	/**
	* Продолжить авторизацию авторизацию
	* @param session сеанс
	* @param input ввод от клиента
	* @param output ответ сервера
	*/
	status_t step(GSASLSession *session, const std::string &input, std::string &output);
	
	/**
	* Вернуть логин пользователя авторизованного
	* @param session сеанс
	*/
	std::string getUsername(GSASLSession *session);
	
	/**
	* Закрыть сеанс
	*/
	void close(GSASLSession *session);
};

#endif // NANOSOFT_GSASLSERVER_H
