# Makefile
# Author: Samuel Dobroň, FIT

LC_ALL=en_GB.UTF-8
CC=gcc
CFLAGS= -O2 -g -std=c11 -pedantic -Wall -Wextra
LDFLAGS=

.PHONY: clean all

all: sender receiver

clean:
	rm {receiv,send}er/*.{o,txt} dns_* 2> /dev/null || true

-include receiver/dependencies.txt
receiver/dependencies.txt:
	$(CC) $(CFLAGS) -MM receiver/*.c > $@

-include sender/dependencies.txt
sender/dependencies.txt:
	$(CC) $(CFLAGS) -MM sender/*.c > $@

sender: sender/dns_sender_events.o sender/sender.o sender/args_parser.o
	$(CC) $(CFLAGS) $^ -o dns_$@

receiver: receiver/dns_receiver_events.o receiver/receiver.o receiver/args_parser.o receiver/connections.o
	$(CC) $(CFLAGS) $^ -o dns_$@
