#pragma once

#include <stdlib.h>
#include <memory_managment/kheap.h>
#include <fs/block_device.h>

typedef struct {
    uint32_t        address;
    BlockDevice_t*  device;
} MemoryDevice_t;

MemoryDevice_t* md_init(uint32_t address, uint32_t size, uint32_t block_size);
void            md_free(MemoryDevice_t* dev);