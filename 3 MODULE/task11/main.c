#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "shm_task.h"

volatile sig_atomic_t keep_running = 1;
int processed_count = 0;

void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
}

void cleanup(int shm_fd, shared_data_t *shm, sem_t *sem_p, sem_t *sem_c) { // функция освобождения ресурсов
    if (shm != MAP_FAILED) munmap(shm, sizeof(shared_data_t));
    if (shm_fd != -1) close(shm_fd);
    
    shm_unlink(SHM_NAME);
    sem_close(sem_p);
    sem_close(sem_c);
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint; // устанавливаем обработчик сигнала
    sigemptyset(&sa.sa_mask); // не блокируем дополнительные сигналы
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL); // регистрируем обработчик сигнала SIGINT

    srand(time(NULL));

    // создаем и настраиваем разделяемую память
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) { // устанавливаем размер сегмента разделяемой памяти
        perror("ftruncate");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    // отображаем сегмент разделяемой памяти в адресное пространство процесса
    shared_data_t *shm = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // создаем и настраиваем семафоры для синхронизации между процессами
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD);
    
    sem_t *sem_p = sem_open(SEM_PARENT, O_CREAT, 0666, 0);
    sem_t *sem_c = sem_open(SEM_CHILD, O_CREAT, 0666, 0);

    if (sem_p == SEM_FAILED || sem_c == SEM_FAILED) {
        perror("sem_open");
        cleanup(shm_fd, shm, sem_p, sem_c);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        cleanup(shm_fd, shm, sem_p, sem_c);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // дочерний
        while (keep_running) {
            if (sem_wait(sem_c) == -1) {
                if (errno == EINTR) continue;
                break;
            }
            if (!keep_running) break;
            
            process_numbers(shm);
            sem_post(sem_p);
        }
        exit(EXIT_SUCCESS);
    } else { // родитель
        while (keep_running) {
            generate_numbers(shm);
            sem_post(sem_c);
            
            if (sem_wait(sem_p) == -1) {
                if (errno == EINTR) break;
                perror("sem_wait");
                break;
            }
            
            printf("Parent: Result min=%d, max=%d\n\n", shm->min, shm->max);
            processed_count++;
            sleep(1);
        }

        printf("\nParent: Received SIGINT. Total processed: %d\n", processed_count);
        
        // Wake up child if it's waiting
        sem_post(sem_c);
        wait(NULL);
        
        cleanup(shm_fd, shm, sem_p, sem_c);
    }

    return 0;
}
