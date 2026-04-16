#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    int fd = open(file_name, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    close(fd);

    key_t key = ftok(file_name, 'S');
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    srand((unsigned int)(time(NULL) ^ getpid()));

    while (1) {
        int count = 1 + rand() % 8;
        char line[256];
        int len = 0;

        for (int i = 0; i < count; ++i) {
            int value = rand() % 1000;
            len += snprintf(line + len, sizeof(line) - (size_t)len, "%d%s", value, (i + 1 == count) ? "\n" : " ");
        }

        if (sem_lock(semid) == -1) {
            perror("sem_lock");
            break;
        }

        FILE *f = fopen(file_name, "a");
        if (f == NULL) {
            perror("fopen");
            sem_unlock(semid);
            break;
        }
        fputs(line, f);
        fclose(f);

        if (sem_unlock(semid) == -1) {
            perror("sem_unlock");
            break;
        }

        sleep(1);
    }

    return 0;
}
