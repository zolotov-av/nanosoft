#ifndef NANOSOFT_XMLNSCONTEXT_H
#define NANOSOFT_XMLNSCONTEXT_H

#include <map>
#include <stack>
#include <string>
#include <nanosoft/xmlnsmanager.h>

namespace nanosoft
{
	/**
	* Вспомогательный класс для чтения/записи XML с поддержкой XML Namaspaces
	*/
	class XMLNSContext
	{
	private:
		
		/**
		* Менеджер XML Namespaces
		*/
		const XMLNSManager *nsm;
		
		/**
		* mapping prefix -> namespace
		*/
		std::map<std::string, std::string> p2ns;
		
		/**
		* mapping namespace -> prefix
		*/
		std::map<std::string, std::string> ns2p;
		
		typedef std::pair<std::string, std::string> prefix_t;
		
		/**
		* Стек переопределений префиксов
		*/
		std::stack<prefix_t> stack;
		
	public:
		/**
		* Конструктор
		*/
		XMLNSContext(const XMLNSManager *nsManager);
		
		/**
		* Деструктор
		*/
		~XMLNSContext();
		
		/**
		* Открыть пространство имен
		* @param prefix
		* @param ns
		*/
		void open(const std::string &prefix, const std::string &ns);
		
		/**
		* Кодировать имя
		* @param name имя узла в том виде, как было прочтено из файла
		* @return нормальзованное имя узла
		*/
		std::string encode(const std::string &name);
		
		/**
		* Декодировать имя
		* @param name нормализованное имя
		* @return имя узла в том виде, как будет записано в файл
		*/
		std::string decode(const std::string &name);
		
		/**
		* Закрыть пространство имен
		*/
		void close();
	};
}

#endif // NANOSOFT_XMLNSCONTEXT_H
