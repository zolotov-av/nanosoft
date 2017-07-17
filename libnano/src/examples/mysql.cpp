#include <stdio.h>
#include <nanosoft/mysql.h>
#include <iostream>

using namespace std;
using namespace nanosoft;

int main(int argc, char **argv)
{
	const char *user = argc > 1 ? argv[1] : "";
	const char *pass = argc > 2 ? argv[2] : "";
	const char *db = argc > 3 ? argv[3] : "test";
	const char *sock = argc > 4 ? argv[4] : "/var/lib/mysql/mysql.sock";
	
	MySQL mysql;
	mysql.connectUnix(sock, db, user, pass);
	cerr << "connected\n";
	
	MySQL::result res = mysql.query("SELECT * FROM xmpp_users WHERE username = '%s'", "alex");
	for(; ! res.eof(); res.next())
	{
		cout << res["username"] << endl;
	}
	res.free();
	
	mysql.close();
	return 0;
}
