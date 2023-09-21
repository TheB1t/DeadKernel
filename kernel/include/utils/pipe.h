#pragma once

#include <utils/common.h>
#include <utils/bool.h>

#include <multitasking/mutex.h>

#define PIPE_SIZE 1024

typedef struct {
    uint8_t buffer[PIPE_SIZE];
    uint32_t read_offset;
    uint32_t write_offset;
    bool is_empty;
    bool is_full;
    mutex_t mutex;
} pipe_t;

pipe_t* pipe_alloc();
void    pipe_free(pipe_t* p);
void    pipe_init(pipe_t* p);
int     pipe_write(pipe_t* p, uint8_t* buf, int size);
int     pipe_read(pipe_t* p, uint8_t* buf, int size);