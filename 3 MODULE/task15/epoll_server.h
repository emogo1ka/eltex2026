#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H

#include <stdio.h>
#include <netinet/in.h>
#include "../task13/protocol.h"

typedef enum {
    STATE_WAIT_CMD,
    STATE_WAIT_MATH_OP,
    STATE_WAIT_MATH_A,
    STATE_WAIT_MATH_B,
    STATE_WAIT_FILENAME,
    STATE_SENDING_FILE
} client_state_t;

typedef struct {
    int fd;
    client_state_t state;
    char math_op;
    char val_buffer[64];
    FILE *file_ptr;
    long file_remaining;
} client_context_t;

#endif
