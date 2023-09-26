#pragma once

#include <utils/common.h>

#include <multitasking/mutex.h>

typedef struct {
    uint8_t* data;

    uint32_t size;
    uint32_t count;
    int32_t front;
    int32_t rear;
} cyclic_buffer_t;

cyclic_buffer_t*    cyclic_buffer_init(uint32_t size);
void                cyclic_buffer_free(cyclic_buffer_t* buffer);
int                 cyclic_buffer_enqueue(cyclic_buffer_t* buffer, uint8_t value);
int                 cyclic_buffer_dequeue(cyclic_buffer_t* buffer, uint8_t* value);
int                 cyclic_buffer_read(cyclic_buffer_t* buffer, uint8_t* value);
void                cyclic_buffer_clear(cyclic_buffer_t* buffer);