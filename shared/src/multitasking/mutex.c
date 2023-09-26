#include <multitasking/mutex.h>

void mutex_init(mutex_t* mutex) {
    uint32_t initial = 1;
    mutex->semaphore_id = semctl(0, SEMAPHORE_INIT, &initial);
}

void mutex_lock(mutex_t* mutex) {
    semctl(mutex->semaphore_id, SEMAPHORE_WAIT, NULL);
}

void mutex_unlock(mutex_t* mutex) {
    semctl(mutex->semaphore_id, SEMAPHORE_SIGNAL, NULL);
}

void mutex_free(mutex_t* mutex) {
    semctl(mutex->semaphore_id, SEMAPHORE_FREE, NULL);
}