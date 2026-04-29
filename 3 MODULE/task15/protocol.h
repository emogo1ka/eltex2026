#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#define DEFAULT_PORT 8080
#define CHUNK_SIZE 4096

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_FILE_TRANSFER,
    OP_RESULT,
    OP_ERROR
} op_type_t;

typedef struct {
    op_type_t type;
    union {
        struct {
            double a;
            double b;
        } math;
        struct {
            char filename[256];
            long size;
        } file_meta;
        struct {
            double value;
        } result;
        char error_msg[256];
    } payload;
} packet_t;

#endif // PROTOCOL_H
