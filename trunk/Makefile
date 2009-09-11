
srcdir = .

all: forker miniget wait time parse_url masterd nsu streams miniget2 options libnanosoft.a
	date "+%d.%m.%Y %H:%M:%S" > build

check: forker
	./time ./forker $(FORKC) ./wait 5

forker: forker.c
	gcc forker.c -o forker

miniget: miniget.c nanourl.h nanourl.c
	gcc miniget.c nanourl.c -o miniget

miniget2: miniget.cpp libnanosoft.a
	gcc miniget.cpp -lnanosoft -lstdc++ -L$(srcdir) -I$(srcdir) -o miniget2

wait: wait.c
	gcc wait.c -o wait

time: time.c
	gcc time.c -o time

parse_url: parse_url.c nanourl.c nanourl.h
	gcc parse_url.c nanourl.c -o parse_url

masterd: masterd.c nanoini.o nanostr.o
	gcc masterd.c nanoini.o nanostr.o -o masterd

nsu: nsu.c
	gcc nsu.c -o nsu

streams: streams.cpp fstream.o
	gcc streams.cpp fstream.o -o streams -I${srcdir} -lstdc++

options: options.o options_test.cpp
	gcc options.o options_test.cpp -I$(srcdir) -lstdc++ -o options

libnanosoft.a: nanoini.o nanostr.o nanourl.o options.o fstream.o socket.o http.o
	ar rc libnanosoft.a nanoini.o nanostr.o nanourl.o options.o fstream.o socket.o http.o

nanoini.o: nanoini.c nanoini.h
	gcc -c nanoini.c

nanostr.o: nanostr.c nanostr.h
	gcc -c nanostr.c

nanourl.o: nanourl.c nanourl.h
	gcc -c nanourl.c

options.o: options.cpp nanosoft/options.h
	gcc -c options.cpp -I$(srcdir)

fstream.o: fstream.cpp nanosoft/stream.h nanosoft/fstream.h
	gcc -c fstream.cpp -I$(srcdir)

socket.o: socket.cpp nanosoft/stream.h nanosoft/socket.h
	gcc -c socket.cpp -I$(srcdir)

http.o: http.cpp nanosoft/stream.h nanosoft/socket.h nanourl.h
	gcc -c http.cpp -I$(srcdir)

clean:
	rm -f *.o
	rm -f build
	rm -f forker miniget wait time parse_url masterd nsu streams miniget2 libnanosoft.a options
