CFLAGS=-pipe -fdiagnostics-color -g -Og -std=c99 -pedantic -Wall -Wextra -Werror -fpic

DEFAULT_GOAL:=all

.PHONY: all buildlib build test cls clean

all: cls test build


cls:
	@cls || clear


buildlib: 
	gcc $(CFLAGS) mparc.c -c

test: buildlib
	gcc $(CFLAGS) tmain.c mparc.o -o tmparc.exe

build: test buildlib
	gcc $(CFLAGS) main.c mparc.o -o mparc.exe


clean:
	rm -rf *.o *.exe *.stackdump *.mpar.* *.mpar