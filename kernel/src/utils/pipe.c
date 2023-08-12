#include <utils/pipe.h>
#include <memory_managment/kheap.h>

pipe_t* pipe_alloc() {
    return (pipe_t*)kmalloc(sizeof(pipe_t));
}

void pipe_free(pipe_t* p) {
    kfree(p);
}

void pipe_init(pipe_t* p) {
    p->read_offset = 0;
    p->write_offset = 0;
    p->is_empty = 1;
    p->is_full = 0;

    mutex_init(&p->mutex);
}

int pipe_write(pipe_t* p, uint8_t* buf, int size) {
    mutex_lock(&p->mutex);

    if (p->is_full)
        goto _error;

    for (int i = 0; i < size; i++) {
        p->buffer[p->write_offset] = buf[i];
        p->write_offset = (p->write_offset + 1) % PIPE_SIZE;
        if (p->write_offset == p->read_offset) {
            p->is_full = 1;
            break;
        }
    }

    p->is_empty = 0;
    mutex_unlock(&p->mutex);
    return size;

_error:
    mutex_unlock(&p->mutex);
    return -1;
}

int pipe_read(pipe_t* p, uint8_t* buf, int size) {
    mutex_lock(&p->mutex);

    if (p->is_empty)
        goto _error;

    int bytes_read = 0;
    while (bytes_read < size && !p->is_empty) {
        buf[bytes_read++] = p->buffer[p->read_offset];
        p->read_offset = (p->read_offset + 1) % PIPE_SIZE;
        if (p->is_full)
            p->is_full = 0;

        if (p->read_offset == p->write_offset)
            p->is_empty = 1;
    }

    mutex_unlock(&p->mutex);
    return bytes_read;

_error:
    mutex_unlock(&p->mutex);
    return -1;
}
