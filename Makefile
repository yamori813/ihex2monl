# Makefile for ihex2mon

CFLAGS=-lm

ihex2monl: main.c readihex.c intel_hex.c intel_hex.h
	${CC} ${CFLAGS} main.c readihex.c intel_hex.c -o $@

test:
	./ihex2monl sample.hex sample.wav

clean:
	rm -f ihex2monl sample.wav
