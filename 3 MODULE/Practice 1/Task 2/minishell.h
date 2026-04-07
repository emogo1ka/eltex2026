#ifndef MINISHELL_H
#define MINISHELL_H

#include <stddef.h>

#define MAX_LINE 512
#define MAX_ARGS 64


int parse_command_line(char *buf, char **argv, int max_args);

int run_command(char **argv);

#endif
