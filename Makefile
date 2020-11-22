CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic -g

all: sps memcheck

sps: sps.o
clean:
	-rm sps sps.o

memcheck:
	-valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./sps CMD_SEQ table_examples/tab1.txt
