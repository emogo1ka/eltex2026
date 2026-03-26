CC = gcc
CFLAGS = -Wall -g -std=c11
TARGET = calculator.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.o calc.o
	$(CC) main.o calc.o -o $(TARGET) -lm -mconsole

main.o: main.c calc.h
	$(CC) $(CFLAGS) -c main.c -o main.o

calc.o: calc.c calc.h
	$(CC) $(CFLAGS) -c calc.c -o calc.o

clean:
	-del /Q /F *.o $(TARGET) 2>nul