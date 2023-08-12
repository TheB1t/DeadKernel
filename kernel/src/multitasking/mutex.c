#include <multitasking/mutex.h>
#include <memory_managment/kheap.h>
#include <multitasking/task.h>

mutex_t* mutex_alloc() {
    return (mutex_t*)kmalloc(sizeof(mutex_t));
}

void mutex_free(mutex_t* mutex) {
    kfree(mutex);
}

void mutex_init(mutex_t* mutex) {
    mutex->locked = false;
}

void mutex_lock(mutex_t* mutex) {
    while (mutex->locked)
        yield();
    mutex->locked = true;
}

void mutex_unlock(mutex_t* mutex) {
    mutex->locked = false;
}