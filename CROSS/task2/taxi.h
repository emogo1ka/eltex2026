#ifndef TASK2_HEADER_H
#define TASK2_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_DRIVERS 100
#define BUFFER_SIZE 256

typedef enum {
    CMD_SEND_TASK,
    CMD_GET_STATUS,
    CMD_GET_DRIVERS,
    CMD_CREATE_DRIVER,
    CMD_QUIT,
    CMD_UNKNOWN
} command_type_t;

typedef struct {
    pid_t pid;
    int req_fd;
    int resp_fd;
    char req_fifo[64];
    char resp_fifo[64];
} driver_info_t;

// Logic functions
void get_fifo_names(pid_t pid, char *req, char *resp);
void driver_loop(pid_t pid);
int parse_command(char *line, int *pid, int *timer);

#endif
