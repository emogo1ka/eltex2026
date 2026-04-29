CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SERVER_TARGET = server
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): server_main.c protocol.h
	$(CC) $(CFLAGS) server_main.c -o $(SERVER_TARGET)

$(CLIENT_TARGET): client_main.c protocol.h
	$(CC) $(CFLAGS) client_main.c -o $(CLIENT_TARGET)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) downloaded_file

.PHONY: all clean
