
#include <nanosoft/switchlogserver.h>

#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

/**
* Конструктор
*/
SwitchLogServer::SwitchLogServer()
{
	int options = 0;
	const char *error;
	int erroffset;
	//
	re = pcre_compile("^[^:]*:\\s*(?:Port\\s+([\\d]+)\\s+link\\s+(up|down)(?:,\\s*(.*))?|Port\\s+(\\d+)\\s+LBD port recovered. Loop detection restarted|Configuration Testing Protocol detects a loop in port\\s+(\\d+))", options, &error, &erroffset, NULL);
	if ( !re )
	{
		fprintf(stderr, "pcre_compile: %s\n", error);
	}
}

/**
* Деструктор
*/
SwitchLogServer::~SwitchLogServer()
{
}

/**
* Вернуть совпадение из PCRE
*/
static void get_pcre_match(const char *data, int *argv, int argc, int offset, char *buf, int buf_len)
{
	if ( offset < 0 || offset >= argc )
	{
		buf[0] = 0;
		return;
	}
	
	int len = argv[offset*2+1] - argv[offset*2];
	if ( len > (buf_len-1) ) len = buf_len-1;
	strncpy(buf, data+argv[offset*2], len);
	buf[len] = 0;
}

/**
* Обработать входящее сообщение
* 
* @param ip адрес источника
* @param port порт источника
* @param data пакет данных
* @param len размер пакета данных
*/
void SwitchLogServer::onRead(const char *ip, int port, const char *data, size_t len)
{
	int ovector[30];
	int count = pcre_exec(re, 0, data, len, 0, 0, ovector, 30);
	if ( count > 0 )
	{
		char port_s[80];
		char state[80];
		char speed[80];
		
		get_pcre_match(data, ovector, count, 2, state, sizeof(state));
		
		if ( strcmp(state, "up") == 0 )
		{
			get_pcre_match(data, ovector, count, 1, port_s, sizeof(port_s));
			get_pcre_match(data, ovector, count, 3, speed, sizeof(speed));
			handlePortUp(ip, atoi(port_s), speed);
			return;
		}
		
		if ( strcmp(state, "down") == 0 )
		{
			get_pcre_match(data, ovector, count, 1, port_s, sizeof(port_s));
			handlePortDown(ip, atoi(port_s));
			return;
		}
		
		get_pcre_match(data, ovector, count, 4, port_s, sizeof(port_s));
		if ( strlen(port_s) > 0 )
		{
			handlePortLoop(ip, atoi(port_s));
			return;
		}
		
		get_pcre_match(data, ovector, count, 5, port_s, sizeof(port_s));
		if ( strlen(port_s) > 0 )
		{
			handlePortLoop(ip, atoi(port_s));
			return;
		}
	}
}

/**
* Сигнал завершения работы
*
* Сервер решил закрыть соединение, здесь ещё есть время
* корректно попрощаться с пиром (peer).
*/
void SwitchLogServer::onTerminate()
{
	fprintf(stderr, "SyslogServer::onTerminate()\n");
	dettach();
}

static string IntToStr(int x)
{
	char buf[80];
	sprintf(buf, "%d", x);
	return buf;
}

/**
* Обработчик включения порта
*
* @param sw_ip IP-адрес коммутатора
* @param port порт на коммутаторе
* @param speed скорость на которой включился порт
*/
void SwitchLogServer::handlePortUp(const char *sw_ip, int port, const char *speed)
{
	printf("switch[%s] port[%d] up, speed[%s]\n", sw_ip, port, speed);
	
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_house_id = "NULL";
	string log_status = "up";
	string log_speed = db.quote(speed);
	
	DB::result r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
			if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
		}
		r.free();
	}
	
	db.query("INSERT INTO switch_port_log (log_switch_ip, log_switch_port, log_switch_id, log_house_id, log_status, log_speed) VALUES (%s, %s, %s, %s, %s, %s)",
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_house_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
}

/**
* Обработчик выключения порта
*
* @param sw_ip IP-адрес коммутатора
* @param port порт на коммутаторе
*/
void SwitchLogServer::handlePortDown(const char *sw_ip, int port)
{
	printf("switch[%s] port[%d] down\n", sw_ip, port);
	
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_house_id = "NULL";
	string log_status = db.quote("down");
	string log_speed = "NULL";
	
	DB::result r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
			if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
		}
		r.free();
	}
	
	db.query("INSERT INTO switch_port_log (log_switch_ip, log_switch_port, log_switch_id, log_house_id, log_status, log_speed) VALUES (%s, %s, %s, %s, %s, %s)",
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_house_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
}

/**
* Обработчик обнаружения петли на порту
*
* @param sw_ip IP-адрес коммутатора
* @param port порт на коммутаторе
*/
void SwitchLogServer::handlePortLoop(const char *sw_ip, int port)
{
	printf("switch[%s] port[%d] loop detected\n", sw_ip, port);
	
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_house_id = "NULL";
	string log_status = db.quote("loop");
	string log_speed = "NULL";
	
	DB::result r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
			if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
		}
		r.free();
	}
	
	db.query("INSERT INTO switch_port_log (log_switch_ip, log_switch_port, log_switch_id, log_house_id, log_status, log_speed) VALUES (%s, %s, %s, %s, %s, %s)",
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_house_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
}
