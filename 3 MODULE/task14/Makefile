CC = gcc
CFLAGS = -Wall -Wextra

TARGET = sniffer
SRCS = main.c sniffer.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) sniff_log.txt

.PHONY: all clean
