LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O0 -g -std=c99 -pedantic -Wall -Wextra -lelf
LDFLAGS=

all: ian-proj1

clean:
	rm src/*.o ian-proj1 ian-proj1.tar.gz 2> /dev/null || true

pack:
	tar -zcvf ian-proj1.tar.gz src/* Makefile

ian-proj1: src
	$(CC) $(CFLAGS) $^ -o $@
