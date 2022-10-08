# Makefile
# Author: Samuel Dobroň, FIT

LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O2 -g -std=c11 -pedantic -Wall -Wextra
LDFLAGS=

.PHONY: clean all

all: sender

clean:
	rm sender/*.o receiver/*.o dns_* sender/receiver 2> /dev/null || true

-include receiver/dependencies.txt
receiver/dependencies.txt:
	$(CC) $(CFLAGS) -MM receiver/*.c > $@

-include sender/dependencies.txt
sender/dependencies.txt:
	$(CC) $(CFLAGS) -MM sender/*.c > $@

sender: sender/dns_sender_events.o sender/sender.o sender/args_parser.o
	$(CC) $(CFLAGS) $^ -o dns_$@
