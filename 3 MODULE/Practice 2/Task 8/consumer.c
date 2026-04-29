#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

static int sem_lock(int semid) {
    struct sembuf op = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    return semop(semid, &op, 1);
}

static int sem_unlock(int semid) {
    struct sembuf op = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    return semop(semid, &op, 1);
}

static int parse_min_max(const char *line, int *min_val, int *max_val) {
    char copy[512];
    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *start = copy;
    if (strncmp(start, "CHECKED|", 8) == 0) start += 8;

    char *save = NULL;
    char *token = strtok_r(start, " \t\r\n", &save);
    if (token == NULL) return -1;

    int value = atoi(token);
    *min_val = value;
    *max_val = value;

    while ((token = strtok_r(NULL, " \t\r\n", &save)) != NULL) {
        value = atoi(token);
        if (value < *min_val) *min_val = value;
        if (value > *max_val) *max_val = value;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    key_t key = ftok(file_name, 'S');
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int semid = semget(key, 1, 0666);
    if (semid == -1) {
        perror("semget (run producer first)");
        return 1;
    }

    printf("Consumer started for file: %s (PID: %d)\n", file_name, getpid());

    while (1) {
        if (sem_lock(semid) == -1) break;

        FILE *f = fopen(file_name, "r");
        if (f == NULL) {
            sem_unlock(semid);
            usleep(500000);
            continue;
        }

        char lines[512][512];
        int line_count = 0;
        while (line_count < 512 && fgets(lines[line_count], sizeof(lines[line_count]), f) != NULL) {
            line_count++;
        }
        fclose(f);

        int target_idx = -1;
        for (int i = 0; i < line_count; ++i) {
            if (strstr(lines[i], "CHECKED|") == NULL && strlen(lines[i]) > 1) {
                target_idx = i;
                break;
            }
        }

        if (target_idx != -1) {
            int min_v, max_v;
            if (parse_min_max(lines[target_idx], &min_v, &max_v) == 0) {
                printf("Consumer [%d]: min=%d max=%d\n", getpid(), min_v, max_v);
            }

            char marked[1024];
            snprintf(marked, sizeof(marked), "CHECKED|%s", lines[target_idx]);
            strncpy(lines[target_idx], marked, sizeof(lines[target_idx]) - 1);
            lines[target_idx][sizeof(lines[target_idx]) - 1] = '\0';

            f = fopen(file_name, "w");
            if (f != NULL) {
                for (int i = 0; i < line_count; ++i) fputs(lines[i], f);
                fclose(f);
            }
        }

        sem_unlock(semid);
        usleep(200000);
    }

    return 0;
}
