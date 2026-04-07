#include "argproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "требуется хотя бы 1 аргумент\n");
        return 1;
    }

    int n = argc - 1;
    int mid = 1 + (n + 1) / 2;
    int parent_start = 1;
    int parent_end = mid - 1;
    int child_start = mid;
    int child_end = argc - 1;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        if (child_start <= child_end)
            process_argument_range(argv, child_start, child_end);
        _exit(0);
    }

    if (parent_start <= parent_end)
        process_argument_range(argv, parent_start, parent_end);

    int st;
    if (waitpid(pid, &st, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    return WIFEXITED(st) ? WEXITSTATUS(st) : 1; // если доч. процесс завершился нормально - возвращаем его код возврата иначе 1
}
