CC = gcc
CFLAGS = -Wall -g -std=c11
TARGET = perm.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.o perm.o
	$(CC) main.o perm.o -o $(TARGET) -mconsole

main.o: main.c perm.h
	$(CC) $(CFLAGS) -c main.c -o main.o

perm.o: perm.c perm.h
	$(CC) $(CFLAGS) -c perm.c -o perm.o

clean:
	-del /Q /F *.o $(TARGET) 2>nul
