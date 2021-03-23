LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -m32 -g -std=c11 -pedantic -Wall -Wextra -lm
LDFLAGS= -m32 -lm

primes.o: primes.c
bitset.o: bitset.c bitset.h
eratosthenes.o: eratosthenes.c eratosthenes.h
error.o: error.c error.h

primes: primes.o bitset.o eratosthenes.o
	$(CC) $(CFLAGS) primes.o bitset.o eratosthenes.o -o primes

run: primes
	-time ./primes

clean:
	rm *.o primes 2> /dev/null || true
