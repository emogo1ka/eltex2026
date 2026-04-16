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

union semun { // структура для управления семафором
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

static int sem_lock(int semid) { 
    // если 0, то процесс будет заблокирован
    struct sembuf op = {.sem_num = 0, .sem_op = -1, .sem_flg = 0}; // оперция -1 для семафора с номером 0
    return semop(semid, &op, 1);
}

static int sem_unlock(int semid) {
    // если 1, то процесс освобождает семафор
    struct sembuf op = {.sem_num = 0, .sem_op = 1, .sem_flg = 0}; // оперция +1 для семафора с номером 0
    return semop(semid, &op, 1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments: %s\n", argv[0]);
        return 1;
    }

    const char *file_name = argv[1];
    int fd = open(file_name, O_CREAT | O_APPEND | O_WRONLY, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    close(fd);

    key_t key = ftok(file_name, 'S'); // младшие биты пути к файлу + младшие биты номера проекта 'S' + номер устройства + номер inode
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666); //создаем новый семафор права: 0666 
    if (semid == -1) {
        if (errno != EEXIST) {
            perror("semget");
            return 1;
        }

        semid = semget(key, 1, 0666); // если семафор уже существует, открываем его для доступа
        if (semid == -1) {
            perror("semget existing");
            return 1;
        }
    } else {
        union semun arg;
        arg.val = 1;
        if (semctl(semid, 0, SETVAL, arg) == -1) {
            perror("semctl");
            return 1;
        }
    }

    srand((unsigned int)(time(NULL) ^ getpid())); // уникальный seed для генератора случайных чисел
    while (1) {
        int count = 1 + rand() % 8; // генерируем от 1 до 8 случайных чисел
        char line[256];
        int len = 0;

        for (int i = 0; i < count; ++i) {
            int value = rand() % 1000; // значения до 1000
            // формируем строку из случайных чисел
            int len = 0;
            for (int i = 0; i < count; i++) {
                len += snprintf(line + len, sizeof(line) - (size_t)len, "%d%s", 
                                values[i], (i + 1 == count) ? "\n" : " ");
            }
        }

        if (sem_lock(semid) == -1) {
            perror("sem_lock");
            break;
        }

        FILE *f = fopen(file_name, "a"); // открываем файл для добавления данных без перезаписи
        if (f == NULL) {
            perror("fopen");
            sem_unlock(semid);
            break;
        }
        fputs(line, f); // записываем строку в файл
        fclose(f);

        if (sem_unlock(semid) == -1) {
            perror("sem_unlock");
            break;
        }

        sleep(1);
    }

    return 0;
}
