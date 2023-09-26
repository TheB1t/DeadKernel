#pragma once

#include <stdlib.h>

typedef struct {
    volatile int32_t value;
} atomic_int_t;


void atomic_init(atomic_int_t *atom, int32_t initial_value);
int32_t atomic_load(const atomic_int_t *atom);
void atomic_store(atomic_int_t *atom, int32_t new_value);
void atomic_increment(atomic_int_t *atom);
void atomic_decrement(atomic_int_t *atom);