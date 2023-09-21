#include <utils/cyclic_buffer.h>
#include <memory_managment/kheap.h>

cyclic_buffer_t* cyclic_buffer_init(uint32_t size) {
    cyclic_buffer_t* buffer = (cyclic_buffer_t*)kmalloc(sizeof(cyclic_buffer_t));

    buffer->data = (uint8_t*)kmalloc(size);

    buffer->size = size;
    buffer->front = 0;
    buffer->rear = -1;
    buffer->count = 0;

    mutex_init(&buffer->mutex);

    return buffer;
}

void cyclic_buffer_free(cyclic_buffer_t* buffer) {
    kfree(buffer->data);
    kfree(buffer);
}

int cyclic_buffer_enqueue(cyclic_buffer_t* buffer, uint8_t value) {
    mutex_lock(&buffer->mutex);

    if (buffer->count < buffer->size) {
        buffer->rear = (buffer->rear + 1) % buffer->size;
        buffer->data[buffer->rear] = value;
        buffer->count++;
    } else goto _error;

    mutex_unlock(&buffer->mutex);
    return 0;
_error:
    mutex_unlock(&buffer->mutex);
    return -1;
}

int cyclic_buffer_dequeue(cyclic_buffer_t* buffer, uint8_t* value) {
    mutex_lock(&buffer->mutex);

    if (buffer->count > 0) {
        *value = buffer->data[buffer->front];
        buffer->front = (buffer->front + 1) % buffer->size;
        buffer->count--;
    } else goto _error;

    mutex_unlock(&buffer->mutex);
    return 0;
_error:
    mutex_unlock(&buffer->mutex);
    return -1;
}

void cyclic_buffer_clear(cyclic_buffer_t* buffer) {
    buffer->front = 0;
    buffer->rear = -1;
    buffer->count = 0;
}