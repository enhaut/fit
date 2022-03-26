LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O0 -g -std=c99 -pedantic -Wall -Wextra -lelf
LDFLAGS=

all: ian-proj1

clean:
	rm src/*.o ian-proj1 xdobro23.zip 2> /dev/null || true

pack:
	tar -zcvf xdobro23.tar.gz src/* Makefile

ian-proj1: src/main.o
	$(CC) $(CFLAGS) $^ -o $@
