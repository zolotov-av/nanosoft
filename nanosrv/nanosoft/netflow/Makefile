# nanosrv NetFlow Makefile

CPPFLAGS=-I../..

libnsrv_netflow.a: netflowcollector.o netflowserver.o ../core/libnsrv_core.a
	ar rc libnsrv_netflow.a netflowcollector.o netflowserver.o
	ranlib libnsrv_netflow.a

netflowcollector.o: netflowcollector.cpp netflowcollector.h netflowtypes.h
	g++ $(CPPFLAGS) -I../core -c netflowcollector.cpp -o netflowcollector.o

netflowserver.o: netflowserver.cpp netflowserver.h netflowcollector.h netflowtypes.h
	g++ $(CPPFLAGS) -I../core -c netflowserver.cpp -o netflowserver.o

../core/libnsrv_core.a:
	make -C ../core

clean:
	rm -f *.a *.o
