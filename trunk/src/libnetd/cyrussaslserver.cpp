
#include <nanosoft/cyrussaslserver.h>
#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#include <stdio.h>
#include <string.h>
#include <exception>
#include <iostream>

using namespace std;

namespace {
	
	int sasl_my_log(void *context __attribute__((unused)),
			int priority,
			const char *message) 
	{
		const char *label;
		
		if (! message)
			return SASL_BADPARAM;
		
		switch (priority) {
		case SASL_LOG_ERR:
			label = "Error";
			break;
		case SASL_LOG_NOTE:
			label = "Info";
			break;
		default:
			label = "Other";
			break;
		}
		
		fprintf(stderr, "SASL %s: %s\n", label, message);
		
		return SASL_OK;
	}
	
	static sasl_callback_t callbacks[] = {
		{SASL_CB_LOG, (int (*)())&sasl_my_log, NULL},
		{SASL_CB_LIST_END, NULL, NULL}
	};
	
}
/**
* Инициализация cyrus-sasl
*/
void CyrusSASLServer::init()
{
	int result = sasl_server_init(callbacks, "sample");
	if (result != SASL_OK) {
		fprintf(stderr, "Cyrus SASL initialization fault\n");
		throw exception();
	}
}

/**
* Финализация cyrus-sasl
*/
void CyrusSASLServer::finish()
{
	sasl_done();
}

/**
* Конструктор
*/
CyrusSASLServer::CyrusSASLServer(const char *service, const char *vhost, const char *realm)
{
	int result = sasl_server_new(service, vhost, realm, 0, 0, 0, 0, &conn);
	if ( result != 0 )
	{
		fprintf(stderr, "Cyrus SASL sasl_server_new fault: %s\n", sasl_errstring(result, NULL, NULL));
		throw exception();
	}
	sasl_security_properties_t props;
	memset(&props, 0L, sizeof(props));
	props.min_ssf = 0;
	props.max_ssf = 0;
	props.maxbufsize = 0;
	props.security_flags = SASL_SEC_NOANONYMOUS | SASL_SEC_NOPLAINTEXT;
	result = sasl_setprop(conn, SASL_SEC_PROPS, &props);
	if (result != SASL_OK) {
		fprintf(stderr, "SASL setting security properties fault\n");
	}
}

/**
* Конструктор
*/
CyrusSASLServer::~CyrusSASLServer()
{
	sasl_dispose(&conn);
}

/**
* Вернуть список методов авторизации
*/
CyrusSASLServer::mechanisms_t CyrusSASLServer::getMechanisms()
{
	const char *data;
	size_t len;
	int count;
	int result = sasl_listmech(conn, 0, 0, ":", 0, &data, &len, &count);
	if (result != SASL_OK)
	{
		fprintf(stderr, "Cyrus SASL sasl_listmech fault: %s\n", sasl_errstring(result, NULL, NULL));
	}
	else
	{
		CyrusSASLServer::mechanisms_t list;
		const char *start = data, *p = data, *limit = data + len;
		while ( start < limit )
		{
			while ( p < limit && *p != ':' ) p++;
			if ( p > start )
			{
				list.push_back( string(start, p) );
			}
			start = ++p;
		}
		return list;
	}
}

/**
* Начать авторизацию
* @param mech выбранный механизм авторизации
* @param input начальный ввод от клиента
* @param output ответ сервера
*/
CyrusSASLServer::status_t CyrusSASLServer::start(const std::string &mech, const std::string &input, std::string &output)
{
	const char *data;
	size_t len;
	int result = sasl_server_start(conn, mech.c_str(), input.c_str(), input.length(), &data, &len);
	switch ( result )
	{
	case SASL_OK:
		return ok;
	case SASL_CONTINUE:
		output.assign(data, len);
		return next;
	default:
		fprintf(stderr, "SASL starting SASL negotiation fault: %s\n", sasl_errstring(result,NULL,NULL));
		return error;
	}
}

/**
* Продолжить авторизацию авторизацию
* @param input ввод от клиента
* @param output ответ сервера
*/
CyrusSASLServer::status_t CyrusSASLServer::step(const std::string &input, std::string &output)
{
	const char *data;
	size_t len;
	int result = sasl_server_step(conn, input.c_str(), input.length(), &data, &len);
	switch ( result )
	{
	case SASL_OK:
		return ok;
	case SASL_CONTINUE:
		output.assign(data, len);
		return next;
	default:
		fprintf(stderr, "SASL starting SASL negotiation fault: %s\n", sasl_errstring(result,NULL,NULL));
		return error;
	}
}
