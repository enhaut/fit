CC=gcc
CFLAGS=-Wall -std=c11 -pedantic -lm
FILES=btree.c ../btree.c ../test_util.c ../test.c

.PHONY: test clean

test: $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES)
	./test

clean:
	rm -f test
