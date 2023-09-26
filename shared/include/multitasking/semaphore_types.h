#pragma once

#include <stdlib.h>

typedef enum {
    SEMAPHORE_INIT      = 0,
    SEMAPHORE_SIGNAL    = 1,
    SEMAPHORE_WAIT      = 2,
    SEMAPHORE_FREE      = 3
} semaphore_commands_t;