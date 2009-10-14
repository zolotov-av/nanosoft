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
	* Обработчик авторизации пользователя
	* @param username логин пользователя
	* @param realm realm...
	* @param password пароль пользователя
	* @return TRUE - авторизован, FALSE - логин или пароль не верен
	*/
	virtual bool onSASLAuthorize(const std::string &username, const std::string &realm, const std::string &password) = 0;
	
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
	virtual status_t start(const std::string &mech, const std::string &input, std::string &output) = 0;
	
	/**
	* Продолжить авторизацию авторизацию
	* @param input ввод от клиента
	* @param output ответ сервера
	*/
	virtual status_t step(const std::string &input, std::string &output) = 0;
};

#endif // NANOSOFT_SASLSERVER_H
