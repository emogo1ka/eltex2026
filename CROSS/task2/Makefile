CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
TARGET = taxi_system

all: $(TARGET)

$(TARGET): main.c taxi_driver.c taxi.h
	$(CC) $(CFLAGS) main.c taxi_driver.c -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f /tmp/taxi_req_* /tmp/taxi_resp_*

# Run: ./taxi_system
