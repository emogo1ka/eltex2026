CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
TARGET = cross1

all: $(TARGET)

$(TARGET): main.c udp_utils.c udp_raw.h
	$(CC) $(CFLAGS) main.c udp_utils.c -o $(TARGET)

clean:
	rm -f $(TARGET)

# Instructions for running:
# Server: sudo ./task1 server 8888
# Client: sudo ./task1 client 127.0.0.1 8888
