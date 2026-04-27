server: server.o
	gcc server.o -o server

server.o: server.c
	gcc -c server.c -o server.o

clean:
	rm --force server server.o
