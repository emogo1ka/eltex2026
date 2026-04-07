CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

TARGET = phonebook
OBJS = main.o phonebook.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o: main.c phonebook.h
phonebook.o: phonebook.c phonebook.h

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

.PHONY: all clean
