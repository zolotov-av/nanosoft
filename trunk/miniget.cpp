
#include <nanosoft/http.h>
#include <iostream>
#include <exception>
#include <string>

using namespace std;
using namespace nanosoft;

string nano_get(const string &URL)
{
	http sock;
	
	if ( ! sock.open(URL.c_str()) )
	{
		cerr << "open URL fault" << endl;
		throw exception();
	}
	
	int len = sock.getContentLength();
	//cout << "Content length: " << len << endl;
	
	char *buf = new char[len+1];
	sock.read(buf, len);
	sock.close();
	
	return string(buf, len);
}

int main(int argc, char **argv)
{
	if ( argc < 2 )
	{
		cout << "miniget2 <URL>" << endl;
		return 0;
	}
	
	string s = nano_get(argv[1]);
	
	cout << s << endl;
	
	return 0;
}
