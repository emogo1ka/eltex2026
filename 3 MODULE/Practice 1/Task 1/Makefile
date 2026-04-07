CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

TARGET = argproc
OBJS = main.o argproc.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

main.o: main.c argproc.h
argproc.o: argproc.c argproc.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
