CFLAGS=-pipe -fdiagnostics-color -g -Og -std=c99 -pedantic -Wall -Wextra -Werror
CLIBS=-ldmallocth

DEFAULT_GOAL:=all

.PHONY: all buildlib build test cls clean mkar

all: cls test build


cls:
	@cls || clear


buildlib: 
	$(CC) $(CFLAGS) mparc.c $(CLIBS) -c

test: buildlib
	$(CC) $(CFLAGS) tmain.c mparc.o $(CLIBS) -o tmparc.exe

build: test buildlib
	$(CC) $(CFLAGS) main.c mparc.o $(CLIBS) -o mparc.exe


clean:
	rm -rf *.o *.exe *.stackdump *.mpar.* *.mpar *.struct *.struct.* *.log *.log.*


mkar:
	./mparc.exe ./my.mpar c ./*.c ./*.h ./*.hpp ./makefile ./*.swigi