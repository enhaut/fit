LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -m32 -g -std=c11 -pedantic -Wall -Wextra -lm
LDFLAGS= -m32 -lm

primes.o: primes.c
bitset.o: bitset.c bitset.h
eratosthenes.o: eratosthenes.c eratosthenes.h
error.o: error.c error.h

ppm.o: ppm.c ppm.h
steg-decode.o: steg-decode.c

primes: primes.o bitset.o eratosthenes.o error.o
steg-decode: ppm.o steg-decode.o error.o eratosthenes.o

run: primes
	-time ./primes

clean:
	rm *.o primes primes-i steg-decode 2> /dev/null || true
