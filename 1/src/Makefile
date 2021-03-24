LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -m32 -g -std=c11 -pedantic -Wall -Wextra -lm
LDFLAGS= -m32 -lm

.PHONY: run clean all

-include dependencies.txt
dependencies.txt:
	$(CC) $(CFLAGS) -MM *.[ch] > $@

all: primes primes-i
run: primes primes-i
	time ./primes
	time ./primes-i

clean:
	rm *.o primes primes-i steg-decode dependencies.txt 2> /dev/null || true

primes-i: primes-i.o bitset-i.o eratosthenes-i.o error.o
primes: primes.o bitset.o eratosthenes.o error.o
steg-decode: ppm.o steg-decode.o error.o eratosthenes.o

# primes-i
%-i.o: %.c
	$(CC) $(CFLAGS) -DUSE_INLINE -c $< -o $@
