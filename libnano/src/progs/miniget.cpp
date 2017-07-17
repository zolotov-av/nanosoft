
#include <nanosoft/http.h>
#include <iostream>
#include <exception>
#include <string>

using namespace std;
using namespace nanosoft;

string nano_get(const string &URL)
{
	http sock;
	string result;
	char buf[4096];
	int rd;
	
	if ( ! sock.open(URL.c_str()) )
	{
		cerr << "open URL fault" << endl;
		throw exception();
	}
	
	while ( rd = sock.read(buf, sizeof(buf)) )
	{
		result.append(buf, rd);
	}
	
	return result;
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
