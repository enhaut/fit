# Makefile
# Řešení IJC-DU1, příklad a), 20.3.2111
# Autor: Samuel Dobroň, FIT
# Přeloženo: gcc 10.2.1

LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -m32 -g -std=c11 -pedantic -Wall -Wextra
LDFLAGS= -m32 -lm
RUNNABLE_TARGETS=dependencies.txt primes primes-i

.PHONY: run clean all

all: $(RUNNABLE_TARGETS) steg-decode
run: $(RUNNABLE_TARGETS)
	time ./primes
	time ./primes-i

clean:
	rm *.o primes primes-i steg-decode dependencies.txt 2> /dev/null || true

-include dependencies.txt
dependencies.txt:
	$(CC) $(CFLAGS) -MM *.c > $@
	grep 'primes.o\|eratosthenes.o' $@ | sed 's/\.o/-i\.o/' >> $@

primes-i: primes-i.o bitset-i.o eratosthenes-i.o error.o
primes: primes.o bitset.o eratosthenes.o error.o
steg-decode: ppm.o steg-decode.o error.o eratosthenes.o

# inline files for primes-i
%-i.o: %.c
	$(CC) $(CFLAGS) -DUSE_INLINE -c $< -o $@
