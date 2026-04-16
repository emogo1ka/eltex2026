CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=-lrt

all: ping pong

ping: ping.c
	$(CC) $(CFLAGS) -o ping ping.c $(LDFLAGS)

pong: pong.c
	$(CC) $(CFLAGS) -o pong pong.c $(LDFLAGS)

clean:
	rm -f ping pong
