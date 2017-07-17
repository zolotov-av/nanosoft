#ifndef NANOSOFT_SASLSERVER_H
#define NANOSOFT_SASLSERVER_H

#include <string>
#include <vector>

/**
* Абстрактный класс серверной реализации SASL
*
* см. http://asg.web.cmu.edu/sasl/
*/
class SASLServer
{
protected:
	/**
	* Вернуть пароль пользователя
	* @param realm домен (виртуальный хост)
	* @param username логин пользователя
	* @return пароль пользователя
	*/
	virtual std::string getUserPassword(const std::string &realm, const std::string &username) = 0;
	
public:
	enum status_t { ok, next, error };
	
	/**
	* Список механизмов авторизации
	*/
	typedef std::vector<std::string> mechanisms_t;
	
	/**
	* Конструктор
	*/
	SASLServer();
	
	/**
	* Конструктор
	*/
	~SASLServer();
	
	/**
	* Вернуть список методов авторизации
	*/
	virtual mechanisms_t getMechanisms() = 0;
	
	/**
	* Начать авторизацию
	* @param mech выбранный механизм авторизации
	* @param input начальный ввод от клиента
	* @param output ответ сервера
	*/
	//virtual status_t start(const std::string &mech, const std::string &input, std::string &output) = 0;
	
	/**
	* Продолжить авторизацию авторизацию
	* @param input ввод от клиента
	* @param output ответ сервера
	*/
	//virtual status_t step(const std::string &input, std::string &output) = 0;
};

#endif // NANOSOFT_SASLSERVER_H
