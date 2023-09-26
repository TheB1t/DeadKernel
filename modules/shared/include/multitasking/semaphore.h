#pragma once

#include <stdlib.h>
#include <multitasking/semaphore_types.h>

uint32_t semctl(uint32_t id, semaphore_commands_t command, void* args);