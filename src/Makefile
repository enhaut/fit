CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -pthread -g
LDFLAGS=

all: dependencies.txt proj2

-include dependencies.txt
dependencies.txt:
	$(CC) $(CFLAGS) -MM *.c > $@

proj2: proj2.o santa.o elf.o reindeer.o
	$(CC) $(CFLAGS) -o $@ $^


clean:
	clear
	rm *.o proj2  dependencies.txt proj2.out 2> /dev/null || true

test: all
	./proj2 3 5 0 0
	sh output_test.sh < proj2.out

	./proj2 20 5 0 0
	sh output_test.sh < proj2.out

	cp proj2 p2test/proj2
	python p2test/p2test.py
