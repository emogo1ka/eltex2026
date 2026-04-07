CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

TARGET = pipeshell
OBJS = main.o pipeshell.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o: main.c pipeshell.h
pipeshell.o: pipeshell.c pipeshell.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
