server: nibble server.o
	gcc server.o base64.o -o server -lssl -lcrypto -Wall

nibble:
	make -C ./NibbleAndAHalf/NibbleAndAHalf/ nibble-and-a-half
	cp ./NibbleAndAHalf/NibbleAndAHalf/libnaah64.a ./
	cp ./NibbleAndAHalf/NibbleAndAHalf/libnaah64.so ./
	cp ./NibbleAndAHalf/NibbleAndAHalf/base64.o ./

server.o: nibble server.c
	gcc -c server.c -o server.o -lssl -lcrypto -Wall

clean:
	rm --force server server.o base64.o libnaah64.a libnaah64.so
	make -C ./NibbleAndAHalf/NibbleAndAHalf/ clean
