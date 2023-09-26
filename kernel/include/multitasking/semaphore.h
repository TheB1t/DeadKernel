#pragma once

#include <stdlib.h>
#include <multitasking/task.h>
#include <multitasking/atomic.h>
#include <multitasking/semaphore_types.h>
#include <llist.h>

typedef struct {
    uint32_t id;
    uint32_t initial_value;
    atomic_int_t count;
    struct task_queue  queue;
    struct list_head   list;
} semaphore_t;

typedef struct {
    struct	list_head   head;
} semaphore_queue_t;

void semctl_init(semaphore_queue_t* sem_queue);
void semctl_process();

uint32_t semctl(uint32_t id, semaphore_commands_t command, void* args);