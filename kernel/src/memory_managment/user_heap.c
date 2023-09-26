#include <memory_managment/user_heap.h>

uint32_t user_malloc(uint32_t size) {
    if (isTaskingInit())
        return heap_malloc(currentTask->heap, size, 0, NULL);

    return 0;
}

void user_free(void* addr) {
    if (isTaskingInit())
        heap_free(currentTask->heap, addr);
}