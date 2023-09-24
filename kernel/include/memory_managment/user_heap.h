#pragma once

#include <utils/common.h>
#include <memory_managment/heap.h>
#include <multitasking/task.h>

uint32_t user_malloc(uint32_t size);
void user_free(void* addr);