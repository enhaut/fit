# Makefile
# Řešení IJC-DU2, 27.3.2021
# Autor: Samuel Dobroň, FIT
# Přeloženo: gcc 10.2.1

LC_ALL=cs_CZ.utf8
CC=gcc
CFLAGS= -O2 -g -std=c11 -pedantic -Wall -Wextra -fPIC
LDFLAGS=

LIBHTAB=htab_init.o htab_bucket_count.o htab_clear.o htab_erase.o htab_find.o htab_for_each.o htab_free.o htab_hash_function.o htab_lookup_add.o htab_move.o htab_size.o

.PHONY: clean all

all: tail wordcount wordcount-dynamic libhtab.a libhtab.so

libhtab.a: $(LIBHTAB)
	ar crs $@ $^

libhtab.so: $(LIBHTAB)
	$(CC) $(CFLAGS) -shared $^ -o $@

clean:
	rm *.o tail wordcount-dynamic wordcount dependencies.txt libhtab.a libhtab.so 2> /dev/null || true

-include dependencies.txt
dependencies.txt:
	$(CC) $(CFLAGS) -MM *.c > $@

tail: tail.o
wordcount: wordcount.o io.o libhtab.a
	$(CC) $(CFLAGS) -o $@ -static  wordcount.o io.o -L. -lhtab

wordcount-dynamic: wordcount.o io.o libhtab.so
	$(CC) $(CFLAGS) -o $@  $^ -L. -lhtab -Wl,-rpath=.
