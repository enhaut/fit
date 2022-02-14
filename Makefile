LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O3 -g -std=c99 -pedantic -Wall -Wextra
LDFLAGS=

all: src/dependencies.txt hinfosvc

clean:
	rm src/*.o hinfosvc src/dependencies.txt 2> /dev/null || true

-include src/dependencies.txt
src/dependencies.txt:
	$(CC) $(CFLAGS) -MM src/*.c > $@

hinfosvc: src/endpoints.o src/main.o src/response.o
	$(CC) $(CFLAGS) $^ -o $@
