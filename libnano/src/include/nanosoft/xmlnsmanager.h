#ifndef NANOSOFT_XMLNSMANAGER_H
#define NANOSOFT_XMLNSMANAGER_H

#include <map>
#include <string>

namespace nanosoft
{
	/**
	* Менеджер XML Namespaces
	*/
	class XMLNSManager
	{
	private:
		/**
		* Информация о пространстве имен
		*/
		struct nsinfo_t
		{
			/**
			* Собственно простанство имен
			*/
			std::string ns;
			
			/**
			* Счетчик ссылок
			*/
			int counter;
			
			nsinfo_t(): counter(1) { }
		};
		
		typedef std::map<std::string, nsinfo_t> p2ns_map;
		typedef std::map<std::string, std::string> ns2p_map;
		
		/**
		* mapping prefix -> namespace
		*/
		p2ns_map p2ns;
		
		/**
		* mapping namespace -> prefix
		*/
		ns2p_map ns2p;
		
	public:
		/**
		* Регистрация стандартного префикса
		* @param prefix префикс
		* @param ns простанство имен
		*/
		void registerPrefix(const std::string &prefix, const std::string &ns);
		
		/**
		* Дерегистрация стандартного префикса
		* @param prefix префикс
		*/
		void unregisterPrefix(const std::string &prefix);
		
		/**
		* Вернуть простанство имен связанное со стандартным префиксом
		* @param prefix префикс
		* @return простраство имен
		*/
		std::string prefixToNS(const std::string &prefix) const;
		
		/**
		* Вернуть стандартный префикс простанства имен
		*
		* Возвращает стандартный префикс который определен для данного NS
		* если стандартного нет, то возвращает '#' + defaultValue,
		* где defaultValue - это то, что предложил парсер, по первому
		* символу # мы отличаем, что это неизвестный нам namespace
		*/
		std::string NSToPrefix(const std::string &uri, const std::string &defaultValue) const;
	};
}

#endif // NANOSOFT_XMLNSMANAGER_H
