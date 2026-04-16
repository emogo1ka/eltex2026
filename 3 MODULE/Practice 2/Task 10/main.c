#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_NUMBERS 32

typedef struct {
    int state; /* 0-empty, 1-data-ready, 2-result-ready, 3-stop */
    int count;
    int numbers[MAX_NUMBERS];
    int min_value;
    int max_value;
    int processed_sets;
} shm_data_t;

static volatile sig_atomic_t stop_requested = 0;

static void handle_sigint(int signum) {
    (void)signum;
    stop_requested = 1;
}

int main(void) {
    signal(SIGINT, handle_sigint);
    srand((unsigned int)(time(NULL) ^ getpid()));

    key_t key = ftok("/tmp", 'M');
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int shmid = shmget(key, sizeof(shm_data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shm_data_t *data = (shm_data_t *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }
    data->state = 0;
    data->processed_sets = 0;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        shmdt(data);
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    if (pid == 0) {
        while (1) {
            if (data->state == 1) {
                int min_v = data->numbers[0];
                int max_v = data->numbers[0];
                for (int i = 1; i < data->count; ++i) {
                    if (data->numbers[i] < min_v) {
                        min_v = data->numbers[i];
                    }
                    if (data->numbers[i] > max_v) {
                        max_v = data->numbers[i];
                    }
                }
                data->min_value = min_v;
                data->max_value = max_v;
                data->state = 2;
            } else if (data->state == 3) {
                break;
            } else {
                usleep(5000);
            }
        }

        shmdt(data);
        _exit(0);
    }

    while (!stop_requested) {
        if (data->state == 0) {
            data->count = 1 + rand() % MAX_NUMBERS;
            for (int i = 0; i < data->count; ++i) {
                data->numbers[i] = rand() % 1000;
            }
            data->state = 1;
        } else if (data->state == 2) {
            printf("Set #%d -> min=%d max=%d\n", data->processed_sets + 1, data->min_value, data->max_value);
            data->processed_sets++;
            data->state = 0;
            usleep(150000);
        } else {
            usleep(5000);
        }
    }

    while (data->state == 1) {
        usleep(5000);
    }
    data->state = 3;

    waitpid(pid, NULL, 0);
    printf("Total processed sets: %d\n", data->processed_sets);

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
