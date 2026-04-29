CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

TARGET = udp_chat
SRCS = main.c udp_chat.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
