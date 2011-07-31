# nanosrv SNMP Makefile

CPPFLAGS=-I../..

libnsrv_snmp.a: asn1reader.o snmpparser.o snmpbuilder.o snmpserver.o ../core/libnsrv_core.a
	ar rc libnsrv_snmp.a asn1reader.o snmpparser.o snmpbuilder.o snmpserver.o
	ranlib libnsrv_snmp.a

asn1reader.o: asn1reader.cpp asn1reader.h
	g++ $(CPPFLAGS) -c asn1reader.cpp -o asn1reader.o

snmpparser.o: snmpparser.cpp snmpparser.h asn1reader.h
	g++ $(CPPFLAGS) -c snmpparser.cpp -o snmpparser.o

snmpbuilder.o: snmpbuilder.cpp snmpbuilder.h
	g++ $(CPPFLAGS) -c snmpbuilder.cpp -o snmpbuilder.o

snmpserver.o: snmpserver.cpp snmpserver.h snmpparser.h asn1reader.h snmpbuilder.h
	g++ $(CPPFLAGS) -c snmpserver.cpp -o snmpserver.o

../core/libnsrv_core.a:
	make -C ../core

clean:
	rm -f *.a *.o
