#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

typedef enum {
    OP_EXIT = 0,
    OP_MATH = 1,
    OP_FILE = 2
} command_t;

#endif
