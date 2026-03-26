CC = gcc
CFLAGS = -Wall -g -std=c11
TARGET = routing.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.o net.o
	$(CC) main.o net.o -o $(TARGET) -mconsole

main.o: main.c net.h
	$(CC) $(CFLAGS) -c main.c -o main.o

net.o: net.c net.h
	$(CC) $(CFLAGS) -c net.c -o net.o

clean:
	-del /Q /F *.o $(TARGET) 2>nul
