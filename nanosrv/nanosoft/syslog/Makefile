# nanosrv syslog Makefile

LIB_CORE=../core/libnsrv_core.a

CPPFLAGS=-I../..

libnsrv_syslog.a: syslogserver.o $(LIB_CORE)
	ar rc libnsrv_syslog.a syslogserver.o
	ranlib libnsrv_syslog.a

syslogserver.o: syslogserver.cpp syslogserver.h
	g++ $(CPPFLAGS) -c syslogserver.cpp -o syslogserver.o

../core/libnsrv_core.a:
	make -C ../core

clean:
	rm -f *.a *.o
