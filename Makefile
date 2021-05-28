fbkeys: fbkeys.c
	mkdir -p bin
	gcc fbkeys.c -o bin/fbkeys

fbkeys.c: fbkeys.lilit
	lilit fbkeys.lilit

clean:
	rm -rf bin
