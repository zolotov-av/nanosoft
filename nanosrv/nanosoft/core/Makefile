# nanosrv core Makefile

CPPFLAGS=-I../..

libnsrv_core.a: object.o netobject.o netdaemon.o udpserver.o
	ar rc libnsrv_core.a object.o netobject.o netdaemon.o udpserver.o
	ranlib libnsrv_core.a

object.o: object.cpp object.h
	g++ $(CPPFLAGS) -c object.cpp -o object.o

netobject.o: netobject.cpp netobject.h object.h
	g++ $(CPPFLAGS) -c netobject.cpp -o netobject.o

netdaemon.o: netdaemon.cpp netdaemon.h netobject.h object.h
	g++ $(CPPFLAGS) -c netdaemon.cpp -o netdaemon.o

udpserver.o: udpserver.cpp udpserver.h netobject.h object.h
	g++ $(CPPFLAGS) -c udpserver.cpp -o udpserver.o

clean:
	rm -f *.a *.o
