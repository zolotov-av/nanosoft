# nanosrv Makefile

LIB_CORE=nanosoft/core/libnsrv_core.a
LIB_SYSLOG=nanosoft/syslog/libnsrv_syslog.a
LIB_NETFLOW=nanosoft/netflow/libnsrv_netflow.a
LIB_SNMP=nanosoft/snmp/libnsrv_snmp.a

CPPFLAGS=-I.

all: demo_syslog demo_netflow demo_snmp

demo_syslog: $(LIB_CORE) $(LIB_SYSLOG) demo_syslog.cpp
	g++ $(CPPFLAGS) -o demo_syslog demo_syslog.cpp $(LIB_CORE) $(LIB_SYSLOG)

demo_netflow: $(LIB_CORE) $(LIB_NETFLOW) demo_netflow.cpp
	g++ $(CPPFLAGS) -o demo_netflow demo_netflow.cpp $(LIB_CORE) $(LIB_NETFLOW)

demo_snmp: $(LIB_CORE) $(LIB_SNMP) demo_snmp.cpp	
	g++ $(CPPFLAGS) -o demo_snmp demo_snmp.cpp $(LIB_CORE) $(LIB_SNMP)

$(LIB_CORE):
	make -C nanosoft/core

$(LIB_SYSLOG):
	make -C nanosoft/syslog

$(LIB_NETFLOW):
	make -C nanosoft/netflow

$(LIB_SNMP):
	make -C nanosoft/snmp

clean:
	rm -f demo_syslog demo_netflow demo_snmp
	make -C nanosoft/core clean
	make -C nanosoft/syslog clean
	make -C nanosoft/netflow clean
	make -C nanosoft/snmp clean
