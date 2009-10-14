#ifndef NANOSOFT_CYRUSSASLSERVER_H
#define NANOSOFT_CYRUSSASLSERVER_H

#include <nanosoft/saslserver.h>
#include <sasl/sasl.h>
#include <vector>
#include <string>

/**
* Абстрактный класс серверной реализации SASL
*
* см. http://asg.web.cmu.edu/sasl/
*/
class CyrusSASLServer: public SASLServer
{
private:
	/**
	* SASL connection
	*/
	sasl_conn_t *conn;
	
	/**
	* Отбработчик авторизации пользователя
	*/
	static int checkUserPassword(sasl_conn_t *conn, CyrusSASLServer *server,
		const char *user, const char *pass, unsigned passlen, struct propctx *propctx);
	
public:
	/**
	* Инициализация cyrus-sasl
	*/
	static void init();
	
	/**
	* Финализация cyrus-sasl
	*/
	static void finish();
	
	/**
	* Конструктор
	*/
	CyrusSASLServer(const char *service, const char *vhost, const char *realm);
	
	/**
	* Конструктор
	*/
	~CyrusSASLServer();
	
	/**
	* Вернуть список методов авторизации
	*/
	mechanisms_t getMechanisms();
	
	/**
	* Начать авторизацию
	* @param mech выбранный механизм авторизации
	* @param input начальный ввод от клиента
	* @param output ответ сервера
	*/
	status_t start(const std::string &mech, const std::string &input, std::string &output);
	
	/**
	* Продолжить авторизацию авторизацию
	* @param input ввод от клиента
	* @param output ответ сервера
	*/
	status_t step(const std::string &input, std::string &output);
	
};

#endif // NANOSOFT_CYRUSSASLSERVER_H
