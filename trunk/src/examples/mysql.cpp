#include <stdio.h>
#include <nanosoft/mysql.h>
#include <iostream>

using namespace std;
using namespace nanosoft;

int main()
{
	MySQL mysql;
	mysql.connectUnix("/var/lib/mysql/mysql.sock", "test", "", "");
	
	MySQL::result res = mysql.query("SELECT * FROM xmpp_users WHERE username = '%s'", "alex");
	for(; ! res.eof(); res.next())
	{
		cout << res["username"] << endl;
	}
	res.free();
	
	mysql.close();
	return 0;
}
