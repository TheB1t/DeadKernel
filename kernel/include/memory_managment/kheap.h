#pragma once

#include <utils/common.h>
#include <memory_managment/heap.h>
#include <memory_managment/kheap.h>

#define KHEAP_START         0xC0000000

extern Heap_t* kernelHeap;

uint32_t	_kmalloc(uint32_t size, uint8_t align, uint32_t* phys);
uint32_t	kmalloc(uint32_t size);
void 		kfree(void* addr);