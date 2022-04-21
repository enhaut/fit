LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O3 -g -std=c99 -pedantic -Wall -Wextra -D_BSD_SOURCE -lpcap
LDFLAGS=

all: src/dependencies.txt ipk-sniffer

clean:
	rm src/*.o ipk-sniffer src/dependencies.txt xdobro23.zip 2> /dev/null || true

-include src/dependencies.txt
src/dependencies.txt:
	$(CC) $(CFLAGS) -MM src/*.c > $@

ipk-sniffer: src/args_parser.o src/main.o src/devicemanager.o
	$(CC) $(CFLAGS) $^ -o $@
