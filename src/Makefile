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
	rm *.o proj2  dependencies.txt 2> /dev/null || true
