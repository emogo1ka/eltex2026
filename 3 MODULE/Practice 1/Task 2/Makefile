CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

MINISHELL = minishell
MINI_OBJS = main.o minishell.o

TOOLS = sumargs joinstr maxnum

all: $(MINISHELL) $(TOOLS)

$(MINISHELL): $(MINI_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MINI_OBJS)

sumargs: sumargs.c
	$(CC) $(CFLAGS) -o $@ sumargs.c

joinstr: joinstr.c
	$(CC) $(CFLAGS) -o $@ joinstr.c

maxnum: maxnum.c
	$(CC) $(CFLAGS) -o $@ maxnum.c

main.o: main.c minishell.h
minishell.o: minishell.c minishell.h

clean:
	rm -f $(MINI_OBJS) $(MINISHELL) sumargs joinstr maxnum

.PHONY: all clean
