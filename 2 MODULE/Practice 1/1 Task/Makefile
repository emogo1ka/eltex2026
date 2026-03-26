CC = gcc
CFLAGS = -Wall -g

.PHONY: all clean

all: phonebook.exe

phonebook.exe: 1task.o phonebook.o
	$(CC) 1task.o phonebook.o -o phonebook.exe

1task.o: 1task.c phonebook.h
	$(CC) $(CFLAGS) -c 1task.c -o 1task.o

phonebook.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) -c phonebook.c -o phonebook.o

clean:
	-del /Q /F *.o phonebook.exe 2>nul