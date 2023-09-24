#pragma once

#include <utils/common.h>
#include <multitasking/task.h>
#include <modules/modules.h>
#include <memory_managment/paging.h>
#include <fs/elf/elf.h>

#define KERNEL_TABLE_OBJ	(&kernelTableObj)

extern ELF32Obj_t		kernelTableObj;

void					initKernelTable(void* sybtabPtr, uint32_t size, uint32_t shindex);
uint8_t					isKernelSectionTableLoaded();

void					stackTrace(uint32_t maxFrames);
