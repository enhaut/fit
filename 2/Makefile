# Makefile
# Řešení IJC-DU2, 27.3.2021
# Autor: Samuel Dobroň, FIT
# Přeloženo: gcc 10.2.1

LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -g -std=c11 -pedantic -Wall -Wextra
LDFLAGS=

.PHONY: run clean all

clean:
	rm *.o tail dependencies.txt 2> /dev/null || true

-include dependencies.txt
dependencies.txt:
	$(CC) $(CFLAGS) -MM *.c > $@

tail: tail.o
