CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lrt -pthread

TARGET = shm_app
SRCS = main.c shm_task.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
