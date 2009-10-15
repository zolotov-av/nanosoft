
#include <nanosoft/gsaslserver.h>
#include <gsasl.h>
#include <stdio.h>
#include <string.h>
#include <exception>
#include <iostream>

using namespace std;

/**
* Класс сеанса
*/
class GSASLSession
{
public:
	GSASLServer *server;
	Gsasl_session *sctx;
	
	/**
	* Конструктор сеанса
	*/
	GSASLSession(GSASLServer *aserver, Gsasl_session *asctx): server(aserver), sctx(asctx)
	{
		//gsasl_session_hook_set(asctx, this);
	}
	
	/**
	* Деструктор сеанса
	*/
	~GSASLSession()
	{
		gsasl_finish(sctx);
	}
};

/**
* Проверка статуса выполенения вызово GNU SASL
*/
void GSASLServer::check(int status)
{
	if ( status != GSASL_OK )
	{
		fprintf(stderr, "GNU SASL initialization failure: %s\n", gsasl_strerror(status));
		throw exception();
	}
}

/**
* Callback для GNU SASL
*/
int GSASLServer::gsaslCallback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop)
{
	switch ( prop )
	{
	case GSASL_PASSWORD:
		{
			GSASLServer *server = (GSASLServer*)gsasl_callback_hook_get(ctx);
			const char *user = gsasl_property_fast(sctx, GSASL_AUTHID);
			const char *realm = gsasl_property_fast(sctx, GSASL_REALM);
			string password = server->getUserPassword(realm, user);
			gsasl_property_set_raw(sctx, GSASL_PASSWORD, password.c_str(), password.length());
			return GSASL_OK;
		}
	default:
		cout << "callback(" << prop << ")\n";
		return GSASL_NO_CALLBACK;
	}
	
}

/**
* Конструктор
*/
GSASLServer::GSASLServer()
{
	check( gsasl_init (&ctx) );
	gsasl_callback_set(ctx, gsaslCallback);
	gsasl_callback_hook_set(ctx, this);
}

/**
* Конструктор
*/
GSASLServer::~GSASLServer()
{
	gsasl_done(ctx);
}

/**
* Вернуть список методов авторизации
*/
SASLServer::mechanisms_t GSASLServer::getMechanisms()
{
	char *data;
	size_t len;
	check( gsasl_server_mechlist(ctx, &data) );
	len = strlen(data);
	SASLServer::mechanisms_t list;
	const char *start = data, *p = data, *limit = data + len;
	while ( start < limit )
	{
		while ( p < limit && *p != ' ' ) p++;
		if ( p > start )
		{
			list.push_back( string(start, p) );
		}
		start = ++p;
	}
	gsasl_free(data);
	return list;
}

/**
* Начать авторизацию
* @param service сервис
* @param host хост
* @param mech выбранный механизм авторизации
*/
GSASLSession* GSASLServer::start(const std::string &service, const std::string &host, const std::string &mech)
{
	Gsasl_session *sctx;
	check( gsasl_server_start(ctx, mech.c_str(), &sctx) );
	gsasl_property_set_raw(sctx, GSASL_SERVICE, service.c_str(), service.length());
	gsasl_property_set_raw(sctx, GSASL_HOSTNAME, host.c_str(), host.length());
	gsasl_property_set_raw(sctx, GSASL_REALM, host.c_str(), host.length());
	return new GSASLSession(this, sctx);
}

/**
* Продолжить авторизацию авторизацию
* @param input ввод от клиента
* @param output ответ сервера
*/
GSASLServer::status_t GSASLServer::step(GSASLSession *session, const std::string &input, std::string &output)
{
	char *data;
	size_t len;
	int status = gsasl_step (session->sctx, input.c_str(), input.length(), &data, &len);
	switch ( status )
	{
	case GSASL_OK:
		output.assign(data, len);
		gsasl_free(data);
		return ok;
	case GSASL_NEEDS_MORE:
		output.assign(data, len);
		gsasl_free(data);
		return next;
	default:
		fprintf(stderr, "GNU SASL starting SASL negotiation fault: %s\n", gsasl_strerror(status));
		return error;
	}
}

/**
* Вернуть логин пользователя авторизованного
* @param session сеанс
*/
std::string GSASLServer::getUsername(GSASLSession *session)
{
	return gsasl_property_fast(session->sctx, GSASL_AUTHID);
}

/**
* Закрыть сеанс
*/
void GSASLServer::close(GSASLSession *session)
{
	delete session;
}
