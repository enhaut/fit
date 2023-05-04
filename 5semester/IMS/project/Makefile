# Makefile
# Author: Samuel Dobroň, FIT

.PHONY: all compile

all: compile

clean:
	rm experiment0 experiment1 2> /dev/null || true

compile:
	g++ src/experiment0.cpp -o experiment0 -lsimlib -Wl,-R/usr/local/lib
	g++ src/experiment1.cpp -o experiment1 -lsimlib -Wl,-R/usr/local/lib

run:
	./experiment0
	./experiment1
