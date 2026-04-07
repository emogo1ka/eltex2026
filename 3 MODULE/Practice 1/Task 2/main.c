#include "minishell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    for (;;) {
        printf("minishell> ");
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) {
            putchar('\n');
            break;
        }

        char *copy = line;
        int argc = parse_command_line(copy, argv, MAX_ARGS);
        if (argc < 0) {
            fputs("Слишком много аргументов.\n", stderr);
            continue;
        }
        if (argc == 0)
            continue;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            if (run_command(argv) < 0) {
                if (errno == ENOENT)
                    fprintf(stderr, "Программа не найдена: %s\n", argv[0]);
                else
                    perror(argv[0]);
            }
            _exit(127);
        }

        int st;
        waitpid(pid, &st, 0);
    }

    return 0;
}
