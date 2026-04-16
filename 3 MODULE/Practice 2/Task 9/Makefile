CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11
LDFLAGS=-pthread

all: task9

task9: main.c
	$(CC) $(CFLAGS) -o task9 main.c $(LDFLAGS)

clean:
	rm -f task9 task9_data.txt
