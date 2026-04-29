CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=-pthread

all: task9

task9: main.c
	$(CC) $(CFLAGS) -o semposix main.c $(LDFLAGS)

clean:
	rm -f semposix semposix_data.txt
