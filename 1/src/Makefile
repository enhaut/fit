LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -m32 -g -std=c11 -pedantic -Wall -Wextra -lm
LDFLAGS= -m32 -lm

# primes
bitset.o: bitset.c bitset.h
eratosthenes.o: eratosthenes.c eratosthenes.h
error.o: error.c error.h

# primes-i
primes-i.o: primes.c
	$(CC) $(CFLAGS) -c $< -o $@
bitset-i.o: bitset.c bitset.h
	$(CC) $(CFLAGS) -c $< -o $@
eratosthenes-i.o: eratosthenes.c eratosthenes.h
	$(CC) $(CFLAGS) -c $< -o $@

# steg-decode
ppm.o: ppm.c ppm.h

primes-i: CFLAGS += -DUSE_INLINE
primes-i: primes-i.o bitset-i.o eratosthenes-i.o error.o
primes: primes.o bitset.o eratosthenes.o error.o
steg-decode: ppm.o steg-decode.o error.o eratosthenes.o

run: primes
	-time ./primes

clean:
	rm *.o primes primes-i steg-decode 2> /dev/null || true
