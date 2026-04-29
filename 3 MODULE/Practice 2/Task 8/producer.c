#define _DEFAULT_SOURCE
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
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    
    // Создаем файл, если его нет
    int fd = open(file_name, O_CREAT | O_WRONLY, 0666);
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

    int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        if (errno != EEXIST) {
            perror("semget");
            return 1;
        }
        semid = semget(key, 1, 0666);
    } else {
        union semun arg;
        arg.val = 1;
        semctl(semid, 0, SETVAL, arg);
    }

    srand((unsigned int)(time(NULL) ^ getpid()));
    printf("Producer started for file: %s (PID: %d)\n", file_name, getpid());

    while (1) {
        int count = 1 + rand() % 8;
        char line[256];
        int offset = 0;

        for (int i = 0; i < count; ++i) {
            int value = rand() % 1000;
            offset += snprintf(line + offset, sizeof(line) - offset, "%d%s", 
                              value, (i + 1 == count) ? "\n" : " ");
        }

        if (sem_lock(semid) == -1) break;

        FILE *f = fopen(file_name, "a");
        if (f != NULL) {
            fputs(line, f);
            fclose(f);
            printf("Producer [%d]: wrote %d numbers\n", getpid(), count);
        }

        sem_unlock(semid);
        sleep(2);
    }

    return 0;
}
