#include "shm_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

void generate_numbers(shared_data_t *shm) {
    shm->count = rand() % (MAX_NUMBERS - 1) + 1; // 1 to MAX_NUMBERS
    printf("Parent: Generating %d numbers: ", shm->count);
    for (int i = 0; i < shm->count; i++) {
        shm->data[i] = rand() % 1000;
        printf("%d ", shm->data[i]);
    }
    printf("\n");
}

void process_numbers(shared_data_t *shm) {
    if (shm->count <= 0) return;
    
    int min = INT_MAX;
    int max = INT_MIN;
    
    for (int i = 0; i < shm->count; i++) {
        if (shm->data[i] < min) min = shm->data[i];
        if (shm->data[i] > max) max = shm->data[i];
    }
    
    shm->min = min;
    shm->max = max;
    printf("Child: Found min=%d, max=%d\n", min, max);
}
