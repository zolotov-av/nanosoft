
#include <nanosoft/switchlogserver.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
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
	re = pcre_compile("(?:Port\\s+([\\d]+)\\s+link\\s+(up|down)(?:,\\s*(.*))?|Port\\s+(\\d+)\\s+LBD port recovered. Loop detection restarted|Configuration Testing Protocol detects a loop in port\\s+(\\d+))", options, &error, &erroffset, NULL);
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
	string packet(data, len);
	printf("SwitchLogServer from[%s]: %s\n", ip, packet.c_str());
	
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
	
	time_t rawtime;
	time (&rawtime);
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_house_id = "NULL";
	string log_event_id = "NULL";
	string log_status = db.quote("up");
	string log_speed = db.quote(speed);
	string house_addr = sw_ip;
	
	DB::result r = db.query("BEGIN");
	if ( r ) r.free();
	
	// ищем коммутатор и информацию о доме
	r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id LEFT JOIN streets ON house_street_id = street_id WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
			if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
			if ( ! r.isNull("house_number") && ! r.isNull("street_name") )
			{
				house_addr = r["street_name"] + " " + r["house_number"];
			}
		}
		r.free();
	}
	
	// ищем событие
	r = db.query("SELECT * FROM events WHERE event_switch_ip = %s AND event_switch_port = %d AND event_status='active' ORDER BY event_regtime ASC LIMIT 1", db.quote(sw_ip).c_str(), port);
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("event_id") )
			{
				log_event_id = db.quote(r["event_id"]);
			}
		}
		r.free();
	}
	
	// вставляем лог-запись
	db.query("INSERT INTO switch_port_log (log_time, log_switch_ip, log_switch_port, log_switch_id, log_house_id, log_event_id, log_status, log_speed) VALUES (%d, %s, %s, %s, %s, %s, %s, %s)",
		rawtime,
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_house_id.c_str(),
		log_event_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
	
	r = db.query("COMMIT");
	if ( r ) r.free();
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
	
	time_t rawtime;
	time (&rawtime);
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_peer_id = "NULL";
	string log_house_id = "NULL";
	string log_event_id = "NULL";
	string log_status = db.quote("down");
	string log_speed = "NULL";
	string house_addr = "?";
	bool haveEvent = false;
	bool switchFound = false;
	bool isMagistral = false;
	
	DB::result r = db.query("BEGIN");
	if ( r ) r.free();
	
	// ищем коммутатор
	r = db.query("SELECT * FROM switches WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			switchFound = true;
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
		}
		r.free();
	}
	
	// ищем активное событие
	r = db.query("SELECT * FROM events WHERE event_switch_ip = %s AND event_switch_port = %d AND event_status='active' ORDER BY event_regtime ASC LIMIT 1", db.quote(sw_ip).c_str(), port);
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("event_id") )
			{
				haveEvent = true;
				log_event_id = db.quote(r["event_id"]);
			}
		}
		r.free();
	}
	
	// проверяем является порт магистральным
	if ( switchFound )
	{
		r = db.query("SELECT * FROM switch_ports WHERE port_switch_id = %s AND port_num = %d ORDER BY port_mtime DESC LIMIT 1",
			log_switch_id.c_str(),
			port
		);
		if ( r )
		{
			if ( ! r.eof() )
			{
				isMagistral = ! r.isNull("port_peer_id");
				if ( isMagistral )
				{
					log_peer_id = db.quote(r["port_peer_id"]);
				}
			}
			r.free();
		}
	}
	
	// Если порт магистральный, а события нет, то создаем
	if ( isMagistral && ! haveEvent )
	{
		// ищем информацию о погасшем коммутаторе и доме
		r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id LEFT JOIN streets ON house_street_id = street_id WHERE switch_id = %s", log_peer_id.c_str());
		if ( r )
		{
			if ( ! r.eof() )
			{
				if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
				if ( ! r.isNull("house_number") && ! r.isNull("street_name") )
				{
					house_addr = r["street_name"] + " " + r["house_number"];
				}
			}
			r.free();
		}
		
		char buf[80];
		struct tm *timeinfo;
		timeinfo = localtime(&rawtime);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
		string ctime = db.quote(buf);
		string title = db.quote("Link down: " + house_addr);
		
		db.query("INSERT INTO events (event_regtime, event_updtime, event_start, event_end, event_title, event_switch_ip, event_switch_port, event_switch_id, event_peer_id, event_house_id) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
			ctime.c_str(),
			ctime.c_str(),
			ctime.c_str(),
			ctime.c_str(),
			title.c_str(),
			log_switch_ip.c_str(),
			log_switch_port.c_str(),
			log_switch_id.c_str(),
			log_peer_id.c_str(),
			log_house_id.c_str()
		);
		log_event_id = IntToStr(db.getLastInsertId());
	}
	
	// вставляем лог-запись
	db.query("INSERT INTO switch_port_log (log_time, log_switch_ip, log_switch_port, log_switch_id, log_peer_id, log_house_id, log_event_id, log_status, log_speed) VALUES (%d, %s, %s, %s, %s, %s, %s, %s, %s)",
		rawtime,
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_peer_id.c_str(),
		log_house_id.c_str(),
		log_event_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
	
	r = db.query("COMMIT");
	if ( r ) r.free();
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
	
	time_t rawtime;
	time (&rawtime);
	string log_switch_ip = db.quote(sw_ip);
	string log_switch_port = IntToStr(port);
	string log_switch_id = "NULL";
	string log_house_id = "NULL";
	string log_event_id = "NULL";
	string log_status = db.quote("loop");
	string log_speed = "NULL";
	string house_addr = sw_ip;
	bool haveEvent = false;
	
	DB::result r = db.query("BEGIN");
	if ( r ) r.free();
	
	// ищем коммутатор и информацию о доме
	r = db.query("SELECT * FROM switches LEFT JOIN houses ON switch_house_id = house_id LEFT JOIN streets ON house_street_id = street_id WHERE switch_ip = %s", db.quote(sw_ip).c_str());
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("switch_id") ) log_switch_id = db.quote(r["switch_id"]);
			if ( ! r.isNull("house_id") ) log_house_id = db.quote(r["house_id"]);
			if ( ! r.isNull("house_number") && ! r.isNull("street_name") )
			{
				house_addr = r["street_name"] + " " + r["house_number"];
			}
		}
		r.free();
	}
	
	// ищем событие
	r = db.query("SELECT * FROM events WHERE event_switch_ip = %s AND event_switch_port = %d AND event_status='active' ORDER BY event_regtime ASC LIMIT 1", db.quote(sw_ip).c_str(), port);
	if ( r )
	{
		if ( ! r.eof() )
		{
			if ( ! r.isNull("event_id") )
			{
				haveEvent = true;
				log_event_id = db.quote(r["event_id"]);
			}
		}
		r.free();
	}
	
	// если события нет, то создаем
	if ( ! haveEvent )
	{
		char buf[80];
		struct tm *timeinfo;
		timeinfo = localtime(&rawtime);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
		string ctime = db.quote(buf);
		
		string title = db.quote("Loop: " + house_addr);
		db.query("INSERT INTO events (event_regtime, event_updtime, event_start, event_end, event_title, event_switch_ip, event_switch_port, event_switch_id, event_peer_id, event_house_id) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
			ctime.c_str(),
			ctime.c_str(),
			ctime.c_str(),
			ctime.c_str(),
			title.c_str(),
			log_switch_ip.c_str(),
			log_switch_port.c_str(),
			log_switch_id.c_str(),
			log_switch_id.c_str(),
			log_house_id.c_str()
		);
		log_event_id = IntToStr(db.getLastInsertId());
	}
	
	// вставляем лог-запись
	db.query("INSERT INTO switch_port_log (log_time, log_switch_ip, log_switch_port, log_switch_id, log_peer_id, log_house_id, log_event_id, log_status, log_speed) VALUES (%d, %s, %s, %s, %s, %s, %s, %s, %s)",
		rawtime,
		log_switch_ip.c_str(),
		log_switch_port.c_str(),
		log_switch_id.c_str(),
		log_switch_id.c_str(),
		log_house_id.c_str(),
		log_event_id.c_str(),
		log_status.c_str(),
		log_speed.c_str()
	);
	
	r = db.query("COMMIT");
	if ( r ) r.free();
}
