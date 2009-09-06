
srcdir = .

all: forker miniget wait time parse_url masterd nsu streams
	date "+%d.%m.%Y %H:%M:%S" > build

check: forker
	./time ./forker $(FORKC) ./wait 5

forker: forker.c
	gcc forker.c -o forker

miniget: miniget.c nanourl.h nanourl.c
	gcc miniget.c nanourl.c -o miniget

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

nanoini.o: nanoini.c nanoini.h
	gcc -c nanoini.c

nanostr.o: nanostr.c nanostr.h
	gcc -c nanostr.c

fstream.o: fstream.cpp nanosoft/stream.h nanosoft/fstream.h
	gcc -c fstream.cpp -I$(srcdir)

clean:
	rm -f *.o
	rm -f build
	rm -f forker miniget wait time parse_url masterd nsu streams
