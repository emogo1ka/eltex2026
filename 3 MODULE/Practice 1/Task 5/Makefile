CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

TARGET = sigcounter
OBJS = main.o counter.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o: main.c counter.h
counter.o: counter.c counter.h

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
