#ifndef SHM_TASK_H
#define SHM_TASK_H

#include <semaphore.h>

#define SHM_NAME "/shared_memory"
#define SEM_PARENT "/sem_parent"
#define SEM_CHILD "/sem_child"
#define MAX_NUMBERS 100

typedef struct {
    int count;
    int data[MAX_NUMBERS];
    int min;
    int max;
} shared_data_t;

void generate_numbers(shared_data_t *shm);
void process_numbers(shared_data_t *shm);

#endif
