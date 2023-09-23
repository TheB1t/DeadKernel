#pragma once

#include <utils/common.h>

typedef struct {
    bool locked;
} mutex_t;

mutex_t*    mutex_alloc();
void        mutex_free(mutex_t* mutex);
void        mutex_init(mutex_t* mutex);
void        mutex_lock(mutex_t* mutex);
void        mutex_unlock(mutex_t* mutex);