#pragma once

#include <stdlib.h>
#include <multitasking/semaphore.h>

typedef struct {
    uint32_t semaphore_id;
} mutex_t;

void        mutex_init(mutex_t* mutex);
void        mutex_lock(mutex_t* mutex);
void        mutex_unlock(mutex_t* mutex);
void        mutex_free(mutex_t* mutex);