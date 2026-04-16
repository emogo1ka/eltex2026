#define _GNU_SOURCE
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int parse_min_max(const char *line, int *min_val, int *max_val) {
    char copy[256];
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *save = NULL;
    char *token = strtok_r(copy, " \t\r\n", &save);
    if (token == NULL) {
        return -1;
    }

    int value = atoi(token);
    *min_val = value;
    *max_val = value;

    while ((token = strtok_r(NULL, " \t\r\n", &save)) != NULL) {
        value = atoi(token);
        if (value < *min_val) {
            *min_val = value;
        }
        if (value > *max_val) {
            *max_val = value;
        }
    }
    return 0;
}

int main(void) {
    const char *file_name = "task9_data.txt";
    const char *sem_name = "/task9_file_sem";

    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        sem_close(sem);
        sem_unlink(sem_name);
        return 1;
    }

    if (pid == 0) {
        while (1) {
            sem_wait(sem);

            FILE *f = fopen(file_name, "r");
            if (f == NULL) {
                sem_post(sem);
                usleep(200000);
                continue;
            }

            char lines[256][256];
            int line_count = 0;
            while (line_count < 256 && fgets(lines[line_count], sizeof(lines[line_count]), f) != NULL) {
                line_count++;
            }
            fclose(f);

            int idx = -1;
            for (int i = 0; i < line_count; ++i) {
                if (strncmp(lines[i], "DONE|", 5) != 0 && lines[i][0] != '\n') {
                    idx = i;
                    break;
                }
            }

            if (idx != -1) {
                int min_v = 0;
                int max_v = 0;
                if (parse_min_max(lines[idx], &min_v, &max_v) == 0) {
                    printf("Child: min=%d max=%d\n", min_v, max_v);
                    fflush(stdout);
                }

                char marked[256];
                snprintf(marked, sizeof(marked), "DONE|%s", lines[idx]);
                strncpy(lines[idx], marked, sizeof(lines[idx]) - 1);
                lines[idx][sizeof(lines[idx]) - 1] = '\0';

                f = fopen(file_name, "w");
                if (f != NULL) {
                    for (int i = 0; i < line_count; ++i) {
                        fputs(lines[i], f);
                    }
                    fclose(f);
                }
            }

            sem_post(sem);
            usleep(200000);
        }
    }

    srand((unsigned int)(time(NULL) ^ getpid()));
    for (int round = 0; round < 10; ++round) {
        int count = 1 + rand() % 8;
        char line[256];
        int len = 0;
        for (int i = 0; i < count; ++i) {
            int value = rand() % 1000;
            len += snprintf(line + len, sizeof(line) - (size_t)len, "%d%s", value, (i + 1 == count) ? "\n" : " ");
        }

        sem_wait(sem);
        FILE *f = fopen(file_name, "a");
        if (f != NULL) {
            fputs(line, f);
            fclose(f);
        }
        sem_post(sem);
        sleep(1);
    }

    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    sem_close(sem);
    sem_unlink(sem_name);
    return 0;
}
