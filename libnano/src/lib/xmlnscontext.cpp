#include <map>
#include <string>
#include <nanosoft/xmlnscontext.h>
#include <nanosoft/error.h>

using namespace std;

namespace nanosoft
{
	/**
	* Конструктор
	*/
	XMLNSContext::XMLNSContext(const XMLNSManager *nsManager): nsm(nsManager)
	{
	}
	
	/**
	* Деструктор
	*/
	XMLNSContext::~XMLNSContext()
	{
	}
	
	/**
	* Открыть пространство имен
	* @param prefix
	* @param ns
	*/
	void XMLNSContext::open(const std::string &prefix, const std::string &ns)
	{
		stack.push(make_pair(prefix, p2ns[prefix]));
		p2ns[prefix] = ns;
		ns2p[ns] = prefix;
	}
	
	/**
	* Кодировать имя
	* @param name имя узла в том виде, как было прочтено из файла
	* @return нормальзованное имя узла
	*/
	std::string XMLNSContext::encode(const std::string &name)
	{
		size_t p = name.find(':');
		if ( p == string::npos )
		{
			return nsm->NSToPrefix(p2ns[""], "") + ":" + name;
		}
		else
		{
			string prefix = name.substr(0, p);
			return nsm->NSToPrefix(p2ns[prefix], prefix) + ":" + name.substr(p+1);
		}
	}
	
	/**
	* Декодировать имя
	* @param name нормализованное имя
	* @return имя узла в том виде, как будет записано в файл
	*/
	std::string XMLNSContext::decode(const std::string &name)
	{
		size_t p = name.find(':');
		if ( p == string::npos )
		{
			fatal_error("[XMLNSContext] encoded name must be prefixed: " + name);
			return name;
		}
		else
		{
			string prefix = name.substr(0, p);
			return ns2p[ nsm->prefixToNS(prefix) ] + ":" + name.substr(p+1);
		}
	}
	
	/**
	* Закрыть пространство имен
	*/
	void XMLNSContext::close()
	{
		prefix_t p = stack.top();
		p2ns[p.first] = p.second;
		ns2p[p.second] = p.first;
		stack.pop();
	}
}
