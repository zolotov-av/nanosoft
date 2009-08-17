
all: forker miniget wait time
	date "+%d.%m.%Y %H:%M:%S" > build

check: forker
	./forker $(FORKC) ./wait 5

forker: forker.c
	gcc forker.c -o forker

miniget: miniget.c
	gcc miniget.c -o miniget

wait: wait.c
	gcc wait.c -o wait

time: time.c
	gcc time.c -o time

clean:
	rm -f *.o
	rm -f build
	rm -f forker miniget wait time

