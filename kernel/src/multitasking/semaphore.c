#include <multitasking/semaphore.h>

static uint32_t nextSemaphoreID = 1;
semaphore_queue_t* semaphore_queue = NULL;

void semctl_init(semaphore_queue_t* sem_queue) {
    INIT_LIST_HEAD(LIST_GET_HEAD(sem_queue));

    semaphore_queue = sem_queue;
}

void semctl_process() {
    if (semaphore_queue == NULL)
        return;

    struct list_head* iter;

    list_for_each(iter, LIST_GET_HEAD(semaphore_queue)) {
        semaphore_t* semaphore = list_entry(iter, semaphore_t, list);

        if (atomic_load(&semaphore->count) > 0 && !list_empty(LIST_GET_HEAD(&semaphore->queue))) {
            Task_t* task = list_entry(LIST_GET_HEAD(&semaphore->queue)->next, Task_t, list);

            serialprintf(COM1, "Semaphore: restore task %d\n", task->id);
            atomic_decrement(&semaphore->count);

            list_move_tail(LIST_GET_LIST(task), LIST_GET_HEAD(&mainQueue));
        }
    }
}

uint32_t semctl(uint32_t id, semaphore_commands_t command, void* args) {
    semaphore_t* semaphore = NULL;

    if (semaphore_queue == NULL)
        goto _error;

    if (id != 0 && command != SEMAPHORE_INIT) {
        struct list_head* iter;

        list_for_each(iter, LIST_GET_HEAD(semaphore_queue)) {
            semaphore_t*  tmp = list_entry(iter, semaphore_t, list);
            if (tmp->id == id) {
                semaphore = tmp;
                break;
            }
        }
    }

    switch (command) {
        case SEMAPHORE_INIT:
            if (args == NULL)
                goto _error;

            uint32_t initial_value;
            copyFromUser(&initial_value, args, sizeof(uint32_t));

            semaphore = (semaphore_t*)kmalloc(sizeof(semaphore_t));

            semaphore->id = nextSemaphoreID++;
            semaphore->initial_value = initial_value;

            INIT_LIST_HEAD(LIST_GET_HEAD(&semaphore->queue));

            atomic_init(&semaphore->count, (int32_t)initial_value);

            list_add_tail(LIST_GET_LIST(semaphore), LIST_GET_HEAD(semaphore_queue));
            break;

        case SEMAPHORE_SIGNAL:
            if (!semaphore)
                goto _error;

            serialprintf(COM1, "Semaphore: got signal from task %d\n", currentTask->id);
            if (atomic_load(&semaphore->count) < initial_value)
                atomic_increment(&semaphore->count);
            break;

        case SEMAPHORE_WAIT:
            if (!semaphore)
                goto _error;

            Task_t* task = currentTask;

            if (atomic_load(&semaphore->count) > 0) {
                serialprintf(COM1, "Semaphore: passing trough task %d\n", task->id);
                atomic_decrement(&semaphore->count);
                break;
            }

            serialprintf(COM1, "Semaphore: moving task %d to wait queue\n", task->id);

            switchTask(getInterruptedContext());

            list_move(LIST_GET_LIST(task), LIST_GET_HEAD(&semaphore->queue));
            break;

        case SEMAPHORE_FREE:
            if (!semaphore)
                goto _error;

            if (atomic_load(&semaphore->count) != semaphore->initial_value || !list_empty(LIST_GET_HEAD(&semaphore->queue)))
                goto _error;

            __list_del_entry(LIST_GET_LIST(semaphore));
            kfree(semaphore);
            break;

        default:
            goto _error;
    }

    return semaphore->id;
_error:
    return -1;
}