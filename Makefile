.POSIX:
.SUFFIXES:
.SUFFIXES: .c .o

CC = gcc
CFLAGS = -D_GNU_SOURCE -std=c99 -pedantic -Wall -g -O2
LDLIBS = -lncurses

type9k: type9k.o efn.o vector.o vhelp.o curse.o
	$(CC) $(CFLAGS) -o $@ type9k.o efn.o vector.o vhelp.o curse.o $(LDLIBS)

type9k.o: type9k.c type9k.h efn.h

efn.o: efn.c efn.h

vector.o: vector.c vector.h
vector.c: vtmpl.c vector.h
	touch vector.c
vector.h: vtmpl.h
	touch vector.h

vhelp.o: vhelp.c vector.h vhelp.h

curse.o: curse.c curse.h vector.h

clean:
	rm -f *.o
	for f in *; do if [[ -x "$$f" ]]; then rm -f "$$f"; fi; done
