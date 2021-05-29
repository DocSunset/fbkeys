include config.mk

fbkeys: fbkeys.c
	mkdir -p bin
	${CC} ${CFLAGS} ${LDFLAGS} fbkeys.c -o bin/fbkeys

debug: fbkeys.c
	mkdir -p bin
	${CC} ${DEBUGFLAGS} ${LDFLAGS} fbkeys.c -o bin/fbkeys

fbkeys.c: fbkeys.lilit
	lilit fbkeys.lilit

clean:
	rm -rf bin
