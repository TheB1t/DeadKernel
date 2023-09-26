#include <multitasking/atomic.h>

void atomic_init(atomic_int_t *atom, int32_t initial_value) {
    atom->value = initial_value;
}

int32_t atomic_load(const atomic_int_t *atom) {
    return atom->value;
}

void atomic_store(atomic_int_t *atom, int32_t new_value) {
    atom->value = new_value;
}

void atomic_increment(atomic_int_t *atom) {
    __asm__ __volatile__(
        "lock incl %0"
        : "=m" (atom->value)
        : "m" (atom->value)
    );
}

void atomic_decrement(atomic_int_t *atom) {
    __asm__ __volatile__(
        "lock decl %0"
        : "=m" (atom->value)
        : "m" (atom->value)
    );
}
